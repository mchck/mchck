require 'log'
require 'serialport'
require 'swd-bitbang'

class MchckBitbangSwd < BitbangSwd
  CMD_HANDSHAKE = "?SWD?"
  CMD_HANDSHAKE_REPLY = "!SWD1"
  CMD_WRITE_WORD = 0x90
  CMD_WRITE_BITS = 0xa0
  CMD_READ_WORD = 0x10
  CMD_READ_BITS = 0x20
  CMD_CYCLE_CLOCK = 0x28

  def initialize(opt = {})
    super
    @outbuf = []

    @dev = SerialPort.new(opt[:dev], 115200)
    @dev.flow_control = SerialPort::NONE

    @dev.write(CMD_HANDSHAKE)
    if @dev.read(CMD_HANDSHAKE_REPLY.length) != CMD_HANDSHAKE_REPLY
      raise RuntimeError, "not a MC HCK SWD bitbang adapter"
    end

    @cur_dir = :out
  end

  def flush!
    return if @outbuf.empty?
    data = @outbuf.pack('C*')
    Log(:phys, 2){ ['flush', hexify(data)] }
    @dev.write(data)
    @outbuf = []
  end

  def write_bytes(bytes)
    bytes = bytes.bytes

    while bytes.count >= 4
      if @cur_dir == :in
        Log(:phys, 2){ 'turning out' }
        @outbuf << CMD_CYCLE_CLOCK
        @cur_dir = :out
      end

      this_word = bytes.slice!(0..3)
      @outbuf << CMD_WRITE_WORD
      @outbuf += this_word
    end

    bytes.each do |b|
      @outbuf << (CMD_WRITE_BITS | (8 - 1))
      @outbuf << b
    end
  end

  def write_bits(byte, len)
    return if len == 0

    @outbuf << (CMD_WRITE_BITS | (len - 1))
    @outbuf << byte.ord
  end

  def write_word_and_parity(word, parity)
    write_bytes([word].pack('V'))
    write_bits(parity, 1)
  end

  def write_cmd(cmd)
    write_bytes(cmd)

    # turn in
    @outbuf << CMD_READ_BITS
    @cur_dir = :in

    # read ack
    @outbuf << (CMD_READ_BITS | (3 - 1))

    _, ack = expect_bytes(2).unpack('CC')
    Log(:phys, 2){ 'read ack: %d' % ack }
    ack
  end

  def read_word_and_parity
    raise RuntimeError, "invalid line direction" if @cur_dir != :in

    @outbuf << CMD_READ_WORD
    @outbuf << CMD_READ_BITS

    data = expect_bytes(5)
    ret, par = data.unpack('VC')
    Log(:phys, 2){ 'read word: %08x, parity %d' % [ret, par] }
    [ret, par]
  end

  def expect_bytes(num)
    flush!
    Log(:phys, 5){ 'expecting %d data bytes' % num }
    data = @dev.read(num)
    Log(:phys, 3){ ['read:', hexify(data)] }
    data
  end
end

if $0 == __FILE__
  s = BusPirateSwd.new(:dev=> ARGV[0])

  s.raw_out(255.chr * 7)
  s.raw_out([0xe79e].pack('v'))
  s.raw_out(255.chr * 7)
  s.raw_out(0.chr)
  s.flush!
  begin
    puts "IDCODE: %08x" % s.transact(0xa5)
  rescue
    s.raw_out(255.chr * 7)
    s.raw_out(0.chr)
    puts "IDCODE(2): %08x" % s.transact(0xa5)
  end
end
