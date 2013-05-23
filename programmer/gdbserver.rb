require 'socket'

require 'kinetis'
require 'log'

class GDBServer
  def initialize(target, *addr)
    @target = target
    @servsock = TCPServer.new(*addr)
  end

  def run_loop
    while true
      @sock = @servsock.accept
      begin
        handle_connection
      rescue EOFError, Errno::EPIPE, Errno::ECONNRESET
        Log(:gdb, 1){ "connection closed" }
      end
      @sock.close
    end
  end

  def handle_connection
    Log(:gdb, 1){ "accepted connection" }
    @data = ""
    @sock.setsockopt(:TCP, :NODELAY, true)
    attach
    while true
      cmd = read_packet
      repl = nil
      begin
        # default to "unknown command", but allow
        # explicit nil (needed by R)
        repl = catch (:gdbreply) do
          dispatch(cmd) || ''
        end
      rescue StandardError => e
        Log(:gdb, 1){ ([e] + e.backtrace).join("\n") }
        repl = 'E01'
      end
      reply repl if repl
    end
  end

  def read_packet
    while !(m = /^(.*[$])([^#]*)[#]..(.*)/.match(@data))
      Log(:gdb, 3){ "data #{@data.unpack('H*').first} no full packet, reading more" }
      @data += @sock.readpartial(1024)
    end
    _, discard, pkt, @data = m.to_a
    @sock.write('+')            # ack
    Log(:gdb, 2){ "received pkt #{pkt}, discarded leading `#{discard}'" }
    pkt
  end

  def reply(pkt)
    csum = 0
    pkt.each_byte{|b| csum += b}
    csum = csum % 256
    fullpkt = "$%s#%02x" % [pkt, csum]
    Log(:gdb, 2){ "sending pkt #{fullpkt}" }
    @sock.write(fullpkt)
    # we just ignore the ack, because we're on TCP
  end

  def dispatch(pkt)
    case pkt
    when '!'
      # extended mode
      'OK'
    when '?'
      # get halt reason
      wait_stop
    when /^c([[:xdigit:]]+)?$/
      # continue [addr]
      continue($1)
    when 'D'
      # detach
      detach
    when 'g'
      # read general registers
      read_registers
    when /^G([[:xdigit:]]+)$/
      # write general registers
      write_registers($1)
    when /^m([[:xdigit:]]+),([[:xdigit:]]+)$/
      # read addr,length memory
      read_mem($1, $2)
    when /^M([[:xdigit:]]+),([[:xdigit:]]+):([[:xdigit:]]+)$/
      # write addr,length:data to memory
      write_mem($1, $3)
    when /^qAttached.*/
      # we always attach to an existing process
      '1'
    when /^qSupported.*/
      'PacketSize=0400;qXfer:features:read+'
    when /^qXfer:features:read:target.xml:([[:xdigit:]]+),([[:xdigit:]]+)$/
      send_tdesc($1, $2)
    when /^R..$/
      # reset system
      reset_system(false)
    when /^s([[:xdigit:]]+)?$/
      # single step [addr]
      single_step($1)
    when /^vAttach;.*$/
      attach
    when /^vRun;$/
      reset_system(true)
    when /^([zZ])([01234]),([[:xdigit:]]+),([[:xdigit:]]*).*/
      # remove/insert breakpoint
      breakpoint($1, $2, $3, $4)
    else
      # error
      Log(:gdb, 1){ "invalid packet `#{pkt}'" }
      ''
    end
  end

  def parse_int(str)
    str.to_i(16)
  end

  def parse_binary(data)
    [data].pack('H*')
  end

  def code_binary(data)
    data.unpack('H*').first
  end

  def attach
    @target.enable_debug!
    @target.catch_vector!(true)
    @target.halt_core!
    @target.enable_breakpoints!
    wait_stop
  end

  def detach
    @target.disable_breakpoints!
    @target.disable_debug!
    @target.continue!
    'OK'
  end

  def continue(addrstr)
    if addrstr
      @target.set_register!(:pc, parse_int(addrstr))
    end
    @target.continue!
    wait_stop
  end

  def read_registers
    code_binary(@target.read_registers)
  end

  def write_registers(regstr)
    @target.write_registers!(parse_binary(regstr))
    'OK'
  end

  def read_mem(addrstr, lenstr)
    code_binary(@target.read_mem(parse_int(addrstr), parse_int(lenstr)))
  end

  def write_mem(addrstr, datastr)
    @target.write_mem(parse_int(addrstr), parse_binary(datastr))
    'OK'
  end

  def reset_system(send_reply)
    @target.reset_system!
    if send_reply
      wait_stop
    else
      # this packet has no reply
      throw :gdbreply, nil
    end
  end

  def single_step(addrstr)
    if addrstr
      @target.set_register!(:pc, parse_int(addrstr))
    end
    @target.single_step!
    wait_stop
  end

  def wait_stop
    while !@target.core_halted?
      begin
        @data += @sock.read_nonblock(1024)
        # check for telnet BREAK sequence
        if @data.include?([255, 243].pack('c*')) || @data.include?([3].pack('c'))
          Log(:gdb, 1){ "received break - stopping core" }
          @target.halt_core!
        end
      rescue IO::WaitReadable
      end
      sleep 0.1
    end
    halt_reason = @target.halt_reason
    Log(:gdb, 2){ "halt reason #{halt_reason}" }
    sig_num = Signal.list[halt_reason.to_s] || Signal.list["INT"]
    pc_num = @target.reg_desc.index{|r| r[:name] == :pc}
    cur_pc = @target.get_register(:pc, true)
    'T%02x%02x:%s;' % [sig_num, pc_num, code_binary(cur_pc)]
  end

  def breakpoint(insertstr, typestr, addrstr, kindstr)
    type = case typestr
           when '0'
             :break_software
           when '1'
             :break_hardware
           when '2'
             :watch_write
           when '3'
             :watch_read
           when '4'
             :watch_access
           end
    kind = parse_int(kindstr)
    addr = parse_int(addrstr)
    case insertstr
    when 'Z'
      @target.add_breakpoint(type, addr, kind)
    when 'z'
      @target.remove_breakpoint(type, addr, kind)
    end
    'OK'
  end

  def send_tdesc(addrstr, lenstr)
    addr, len = parse_int(addrstr), parse_int(lenstr)
    desc = @target.tdesc[addr,len]
    if desc.empty?
      'l'
    else
      "m#{desc}"
    end
  end
end

if $0 == __FILE__
  adiv5 = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  k = Kinetis.new(adiv5, false)
  g = GDBServer.new(k, 1234)
  g.run_loop
end
