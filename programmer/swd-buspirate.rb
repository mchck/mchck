require 'log'
require 'serialport'
require 'swd-bitbang'

# Documentation:
# <http://dangerousprototypes.com/docs/Bitbang>
# <http://dangerousprototypes.com/docs/Raw-wire_(binary)>
# Bus Pirate code SVN <https://code.google.com/p/dangerous-prototypes-open-hardware/source/browse/trunk/Bus_Pirate/Firmware>

class BusPirateSwd < BitbangSwd
  CMD_READ_BYTE = 0x06
  CMD_READ_BIT = 0x07
  CMD_WRITE_BYTES = 0x10
  CMD_CLOCK_TICKS = 0x20
  CMD_WRITE_BITS= 0x30
  CMD_CONFIG_PERIPH = 0x40
  CMD_SET_SPEED = 0x60
  CMD_SET_CONFIG = 0x80

  def initialize(opt = {})
    super
    @outbuf = []
    @acks = 0

    @dev = SerialPort.new(opt[:dev], 115200)
    @dev.flow_control = SerialPort::NONE

    # # exit all menus
    # @dev.write("\n" * 10)
    # # reset
    # @dev.write("#")
    # switch to binary mode
    Log(:phys, 1){ "initializing bus pirate" }
    Log(:phys, 2){ "setting binary mode" }
    data = ""
    tries = 0
    # 20 times 0 should get us to binary mode
    while data[-5..-1] != "BBIO1"
      tries += 1
      Log(:phys, 2){ "waiting for BBIO1" }
      @dev.write(0.chr)
      sleep(0.1) if tries == 1 || tries >= 20
      begin
        data << @dev.read_nonblock(1000)
        Log(:phys, 3){ "data: %s" % data }
      rescue IO::WaitReadable
      end
    end
    # switch to raw mode
    Log(:phys, 2){ "setting raw mode" }
    @dev.write(0b00000101.chr)
    while data[-4..-1] != "RAW1"
      Log(:phys, 2){ "waiting for RAW1" }
      sleep(0.1)
      begin
        data << @dev.read_nonblock(1000)
        Log(:phys, 3){ "data: %s" % data }
      rescue IO::WaitReadable
      end
    end
    Log(:phys, 2){ "configuring lines" }
    # set LSB
    @outbuf << (CMD_SET_CONFIG | 0b1010)
    @acks += 1
    # set speed, seems to be required
    @outbuf << (CMD_SET_SPEED | 0b0011)
    @acks += 1
    # enable power
    @outbuf << (CMD_CONFIG_PERIPH | 0b1000)
    @acks += 1
    flush!

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
    bytes = bytes.bytes.to_a

    while !bytes.empty?
      if @cur_dir == :in
        Log(:phys, 2){ 'turning out' }
        @outbuf << (CMD_CLOCK_TICKS | 1)
        @acks += 1
        @cur_dir = :out
      end

      len = bytes.length - 1

      this_bytes = bytes.slice!(0..15)
      @outbuf << (CMD_WRITE_BYTES | (this_bytes.length - 1))
      @outbuf += this_bytes
      @acks += this_bytes.length + 1
    end
  end

  def write_bits(byte, len)
    return if len == 0

    # swap byte order - our interface is LSB first, buspirate does MSB first.
    byte = ("" << byte).unpack('b*').pack('B*')

    @outbuf << (CMD_WRITE_BITS | (len - 1))
    @outbuf << byte.ord
    @acks += 2                  # yes, two acks.
  end

  def write_word_and_parity(word, parity)
    write_bytes([word].pack('V'))
    write_bits(parity, 1)
  end

  def write_cmd(cmd)
    write_bytes(cmd)

    # the buspirate reads _after_ asserting CLK, so we need to skip
    # the turn here and read directly.  We will be one clock short, so
    # when we turn back to write, we will have to clock two periods.

    @cur_dir = :in

    # read ack
    3.times do
      @outbuf << CMD_READ_BIT
    end

    ack = 0
    expect_bytes(3).each_byte.each_with_index do |c, i|
      ack |= (c << i)
    end
    Log(:phys, 2){ 'read ack: %d' % ack }
    ack
  end

  def read_word_and_parity
    raise RuntimeError, "invalid line direction" if @cur_dir != :in

    4.times do
      @outbuf << CMD_READ_BYTE
    end
    @outbuf << CMD_READ_BIT
    flush!

    data = expect_bytes(5)
    ret, par = data.unpack('VC')
    Log(:phys, 2){ 'read word: %08x, parity %d' % [ret, par] }
    [ret, par]
  end

  def expect_bytes(num)
    flush!
    Log(:phys, 5){ 'expecting %d = %d ack + %d data bytes' % [@acks + num, @acks, num] }
    data = @dev.read(@acks + num)
    Log(:phys, 4){ 'skipping %d acks: %s' % [@acks, hexify(data[0...@acks])] }
    data.slice!(0...@acks)
    @acks = 0
    Log(:phys, 3){ ['read:', hexify(data)] }
    # begin
    #   d = @dev.read_nonblock(10)
    #   Log(:phys, 1){ ['found unexpected byte:', hexify(d)] }
    # rescue
    # end
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
