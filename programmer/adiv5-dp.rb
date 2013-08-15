require 'register'
require 'log'

class Adiv5
  class DP
    include Peripheral

    register :ABORT, 0 do
      bool :ORUNERRCLR, 4
      bool :WDERRCLR, 3
      bool :STKERRCLR, 2
      bool :STKCMPCLR, 1
      bool :DAPABORT, 0
    end

    register :CTRLSTAT, 4 do
      bool :CSYSPWRUPACK, 31
      bool :CSYSPWRUPREQ, 30
      bool :CDBGPWRUPACK, 29
      bool :CDBGPWRUPREQ, 28
      bool :CDBGRSTACK, 27
      bool :CDBGRSTREQ, 26
      unsigned :TRNCNT, 21..12
      unsigned :MASKLANE, 11..8
      bool :WDATAERR, 7
      bool :READOK, 6
      bool :STICKYERR, 5
      bool :STICKYCMP, 4
      unsigned :TRNMODE, 3..2
      bool :STICKYORUN, 1
      bool :ORUNDETECT, 0
    end

    register :SELECT, 8 do
      unsigned :APSEL, 31..24
      unsigned :APBANKSEL, 7..4
      bool :CTRLSEL, 0
    end

    unsigned :RDBUF, 12

    def initialize(dp_lower)
      @lower = dp_lower
    end

    def ap_select(apsel, addr)
      apbanksel = addr >> 4
      return if @last_select == [apsel, apbanksel]
      Log(:dp, 3){ "selecting %d:%x" % [apsel, apbanksel] }
      self.SELECT.transact do |select|
        select.zero!
        select.APSEL = apsel
        select.APBANKSEL = apbanksel
      end
      @last_select = [apsel, apbanksel]
    end

    def read(port, addr, opt={})
      v = @lower.read(port, addr, opt)
      Log(:dp, 2){ "read %s %08x < %s" % [port, addr, Log.hexary(v)] }
      v
    end

    def write(port, addr, val)
      Log(:dp, 2){ "write %s %08x = %s" % [port, addr, Log.hexary(val)] }
      @lower.write(port, addr, val)
    end

    def get_backing(offset)
      read(:dp, offset)
    end

    def set_backing(offset, val)
      write(:dp, offset, val)
    end
  end
end
