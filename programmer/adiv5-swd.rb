require 'swd-ftdi'
require 'log'

class Adiv5Swd
  ABORT = 0
  SELECT = 8
  RESEND = 8


  def initialize(drv, opt)
    @drv = drv.new opt

    switch_to_swd
    write_dp(ABORT, 0x1e)           # clear all errors
    write_dp(SELECT, 0)             # select DP bank 0
    @drv.flush!
  end

  def switch_to_swd
    @drv.raw_out(255.chr * 7)        # at least 50 high
    @drv.raw_out([0xe79e].pack('v')) # magic number
    reset
  end

  def reset
    @drv.raw_out(255.chr * 7)        # at least 50 high
    @drv.raw_out(0.chr)              # at least 1 low
    begin
      @drv.transact(0xa5)       # read DPIDR
    rescue
      # If we fail, try again.  We might have been in an unfortunate state.
      @drv.raw_out(255.chr * 7) # at least 50 high
      @drv.raw_out(0.chr)       # at least 1 low
      @drv.transact(0xa5)       # read DPIDR
    end
  end

  def read_ap(addr)
    transact(:ap, :in, addr)
    read_dp(RESEND)
  end

  def write_ap(addr, val)
    transact(:ap, :out, addr, val)
  end

  def read_dp(addr)
    Debug 'read dp %x' % addr
    ret = transact(:dp, :in, addr)
    Debug '==> %08x' % ret
    ret
  end

  def write_dp(addr, val)
    Debug 'write dp %x = %08x' % [addr, val]
    transact(:dp, :out, addr, val)
  end

  def transact(port, dir, addr, data=nil)
    cmd = 0x81
    case port
    when :ap
      cmd |= 0x2
    end
    case dir
    when :in
      cmd |= 0x4
    end
    cmd |= ((addr & 0xc) << 1)
    parity = cmd
    parity ^= parity >> 4
    parity ^= parity >> 2
    parity ^= parity >> 1
    if parity & 1 != 0
      cmd |= 0x20
    end
    @drv.transact(cmd, data)
  end
end


if $0 == __FILE__
  s = Adiv5Swd.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]))
  
end
