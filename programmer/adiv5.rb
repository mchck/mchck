require 'adiv5-swd'
require 'log'

class Adiv5
  ABORT = 0
  CTRLSTAT = 4

  CDBGRSTREQ = 1 << 26
  CDBGRSTACK = 1 << 27
  CDBGPWRUPREQ = 1 << 28
  CDBGPWRUPACK = 1 << 29
  CSYSPWRUPREQ = 1 << 30
  CSYSPWRUPACK = 1 << 31

  SELECT = 8
  RDBUF = 12

  def initialize(drv, opt)
    @dp = Adiv5Swd.new(drv, opt)

    # power up syste + debug
    @dp.write_dp(CTRLSTAT, CDBGPWRUPREQ | CSYSPWRUPREQ)
    waitflags = CDBGPWRUPACK | CSYSPWRUPACK
    while @dp.read_dp(CTRLSTAT) & waitflags != waitflags
      sleep 0.01
    end

    # reset debug
    @dp.write_dp(CTRLSTAT, @dp.read_dp(CTRLSTAT) | CDBGRSTREQ)
    waitflags = CDBGRSTACK
    while @dp.read_dp(CTRLSTAT) & waitflags != waitflags
      sleep 0.01
    end

    # clear reset
    @dp.write_dp(CTRLSTAT, @dp.read_dp(CTRLSTAT) & ~CDBGRSTREQ)
    while @dp.read_dp(CTRLSTAT) & waitflags == waitflags
      sleep 0.01
    end

    Debug "all systems up"
  end
end


if $0 == __FILE__
  p = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  
end
