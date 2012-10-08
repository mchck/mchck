require 'swd-ftdi'
require 'log'

class Adiv5Swd
  ABORT = 0
  SELECT = 8
  RESEND = 8
  RDBUFF = 12

  class ProtocolError < RuntimeError
  end

  class ParityError < RuntimeError
  end

  class Wait < RuntimeError
  end

  class Fault < RuntimeError
  end


  def initialize(drv, opt)
    @drv = drv.new opt

    switch_to_swd
    write(:dp, ABORT, 0x1e)           # clear all errors
    write(:dp, SELECT, 0)             # select DP bank 0
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
    @drv.flush!
    begin
      @drv.transact(0xa5)       # read DPIDR
    rescue
      # If we fail, try again.  We might have been in an unfortunate state.
      @drv.raw_out(255.chr * 7) # at least 50 high
      @drv.raw_out(0.chr)       # at least 1 low
      @drv.transact(0xa5)       # read DPIDR
    end
  end

  def read(port, addr, opt={})
    readcount = opt[:count] || 1
    ret = []

    readcount.times do |i|
      Debug 'read %s %x' % [port, addr]
      ret << transact(port, :in, addr)
    end
    # reads to the AP are posted, so we need to get the result in a
    # separate transaction.
    if port == :ap
      # first discard the first bogus result
      ret.shift
      # add last posted result
      ret << transact(:dp, :in, RDBUFF)
    end
    Debug '==>', *ret.map{|e| "%08x" % e}

    ret = ret.first if not opt[:count]
    ret
  end

  def write(port, addr, val)
    val = [val] unless val.respond_to? :each
    Debug 'write %s %x =' % [port, addr], *val.map{|e| "%08x" % e}
    val.each do |v|
      transact(port, :out, addr, v)
    end
  end

  def transact(port, dir, addr, data=nil)
    try ||= 0
    try += 1

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
  rescue Wait
    Debug 'SWD WAIT, retrying'
    retry
  rescue ProtocolError
    if try <= 3
      Log 'SWD protocol error, retrying'
      reset
      retry
    else
      Log 'SWD protocol error unrecoverable, aborting'
      raise
    end
  rescue ParityError
    Log 'SWD parity error, restarting'
    if port == :ap || addr == RDBUFF
      # If this transfer read from the AP, we have to read from RESEND
      # instead.
      read(:dp, RESEND)
    else
      # We can repeat simple DP reads
      retry
    end
  end
end


if $0 == __FILE__
  s = Adiv5Swd.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]))
  
end
