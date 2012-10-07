require 'adiv5-swd'
require 'log'

class Adiv5
  class DP
    ABORT = 0
    CTRLSTAT = 4

    CDBGRSTREQ = 1 << 26
    CDBGRSTACK = 1 << 27
    CDBGPWRUPREQ = 1 << 28
    CDBGPWRUPACK = 1 << 29
    CSYSPWRUPREQ = 1 << 30
    CSYSPWRUPACK = 1 << 31

    SELECT = 8

    class APSEL
      def self.[](idx)
        idx << 24
      end
    end
    class APBANKSEL
      def self.[](idx)
        idx << 4
      end
    end

    RDBUF = 12
  end

  class AP
    CSW = 0x00
    TAR = 0x04
    DRW = 0x0c
    class BD
      def self.[](idx)
        0x10 + idx * 4
      end
    end
    CFG = 0xf4
    BASE = 0xf8
    IDR = 0xfc


    def self.probe(dp, apsel)
      ap = new(dp, apsel)
      return nil if ap.id == 0
      ap
    rescue
      nil
    end


    attr_reader :id

    def initialize(dp, apsel)
      @dp, @apsel = dp, apsel

      @id = read(IDR)
    end

    def write(addr, val)
      select addr
      @dp.write(:ap, addr & 0xc, val)
    end

    def read(addr)
      select addr
      @dp.read(:ap, addr & 0xc)
    end

    def select(addr)
      @dp.write(:dp, DP::SELECT, DP::APSEL[@apsel] | DP::APBANKSEL[addr >> 4])
    end
  end

  def initialize(drv, opt)
    @dp = Adiv5Swd.new(drv, opt)

    # power up syste + debug
    @dp.write(:dp, DP::CTRLSTAT, DP::CDBGPWRUPREQ | DP::CSYSPWRUPREQ)
    waitflags = DP::CDBGPWRUPACK | DP::CSYSPWRUPACK
    while @dp.read(:dp, DP::CTRLSTAT) & waitflags != waitflags
      sleep 0.01
    end

    # reset debug
    @dp.write(:dp, DP::CTRLSTAT, @dp.read(:dp, DP::CTRLSTAT) | DP::CDBGRSTREQ)
    waitflags = DP::CDBGRSTACK
    while @dp.read(:dp, DP::CTRLSTAT) & waitflags != waitflags
      sleep 0.01
    end

    # clear reset
    @dp.write(:dp, DP::CTRLSTAT, @dp.read(:dp, DP::CTRLSTAT) & ~DP::CDBGRSTREQ)
    while @dp.read(:dp, DP::CTRLSTAT) & waitflags == waitflags
      sleep 0.01
    end

    Debug "all systems up"
  end

  def scan
    256.times do |apsel|
      ap = AP.probe(@dp, apsel)
      if ap
        Debug "found AP %d: %08x" % [apsel, ap.id]
      else
        Debug "no AP on #{apsel}"
      end
    end
  end
end


if $0 == __FILE__
  p = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  p.scan
end
