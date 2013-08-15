require 'adiv5-swd'
require 'log'

class BitbangSwd
  # We need to write on the negative edge, i.e. before asserting CLK.
  # We need to read on the positive edge, i.e. after having asserted CLK.

  ACK_OK = 1
  ACK_WAIT = 2
  ACK_FAULT = 4

  def initialize(opt = {})
    @opt = opt
  end

  def raw_out(seq, seqlen=nil)
    seqlen ||= seq.length * 8
    Log(:phys, 1){ "swd raw: #{seq.unpack("B#{seqlen}").first}" }
    if seqlen >= 8
      write_bytes(seq[0..(seqlen / 8)])
    end
    if seqlen % 8 > 0
      write_bits(seq[-1], seqlen % 8)
    end
  end

  def transact(cmd, data=nil)
    Log(:phys, 1){ 'transact %08b' % cmd }
    case cmd & 0x4
    when 0
      dir = :out
    else
      dir = :in
    end

    ack = write_cmd cmd.chr

    case ack
    when ACK_OK
      # empty
    when ACK_WAIT
      raise Adiv5Swd::Wait
    when ACK_FAULT
      raise Adiv5Swd::Fault
    else
      # we read data right now, just to make sure that we will never
      # work against the protocol
      if dir == :in
        data, par = read_word_and_parity
      end

      raise Adiv5Swd::ProtocolError
    end

    case dir
    when :out
      write_word_and_parity(data, calc_parity(data))
      nil
    when :in
      data, par = read_word_and_parity
      cal_par = calc_parity data
      if par != cal_par
        raise Adiv5Swd::ParityError
      end
      data
    end
  end

  def calc_parity(data)
    data ^= data >> 16
    data ^= data >> 8
    data ^= data >> 4
    data ^= data >> 2
    data ^= data >> 1
    data & 1
  end

  def hexify(str)
    str.unpack('C*').map{|e| "%02x" % e}.join(' ')
  end
end
