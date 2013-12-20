require 'log'
require 'adiv5-swd'
require 'adiv5-dp'
require 'adiv5-ap'

class Adiv5
  def initialize(drv)
    @dp = DP.new(Adiv5Swd.new(drv))

    # power up syste + debug
    @dp.CTRLSTAT.transact do |dp|
      dp.zero!
      dp.CDBGPWRUPREQ = true
      dp.CSYSPWRUPREQ = true
    end
    while true
      ctrlstat = @dp.CTRLSTAT.dup
      break if ctrlstat.CDBGPWRUPACK && ctrlstat.CSYSPWRUPACK
      sleep 0.01
    end

    # don't reset debug, not supposed to be used automatically:
    # http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka14761.html

    # # reset debug
    # @dp.write(:dp, DP::CTRLSTAT, @dp.read(:dp, DP::CTRLSTAT) | DP::CDBGRSTREQ)
    # waitflags = DP::CDBGRSTACK
    # while @dp.read(:dp, DP::CTRLSTAT) & waitflags != waitflags
    #   sleep 0.01
    # end

    # # clear reset
    # @dp.write(:dp, DP::CTRLSTAT, @dp.read(:dp, DP::CTRLSTAT) & ~DP::CDBGRSTREQ)
    # while @dp.read(:dp, DP::CTRLSTAT) & waitflags == waitflags
    #   sleep 0.01
    # end

    Log(:ap, 1){ "all systems up" }
  end

  def ap(idx)
    ap = AP.probe(@dp, idx)
  end

  def dap
    return @dap if @dap
    256.times do |apsel|
      ap = self.ap(apsel)
      if ap
        Log(:ap, 1){ "found AP #{apsel}, #{ap.IDR}, mem: #{ap.mem?}" }
        if !@dap && ap.mem?
          @dap = ap
          break;
        end
      else
        Log(:ap, 1){ "no AP on #{apsel}, stopping probe" }
        break
      end
    end
    @dap
  end

  def idcode
    return @dp.IDCODE
  end
end


if $0 == __FILE__
  require 'backend-driver'
  p = Adiv5.new(BackendDriver.from_string(ARGV[0]))
  require 'pp'
  pp p.dap.devs
end
