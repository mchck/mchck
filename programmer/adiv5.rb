require 'log'
require 'adiv5-swd'
require 'adiv5-dp'
require 'adiv5-ap'

class Adiv5
  attr_reader :ap
  attr_reader :dap

  def initialize(drv, opt)
    @dp = DP.new(Adiv5Swd.new(drv, opt))

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

    @ap = []
    Log :ap, 1, "all systems up"
  end

  def probe
    # Kinetis hack: hold system & core in reset, so that flash and
    # security will have a chance to init, and we have a chance to
    # access the system.  If we don't hold the system in reset, it
    # might loop resetting itself.  While the system resets, it will
    # block debugger access until it has read the security bits.  If
    # the core loops in reset (e.g. because of empty flash), we will
    # get kicked in the nuts regularly.  Holding the system & core in
    # reset prevents this.
    mdmap = AP.probe(@dp, 1)
    if mdmap && mdmap.IDR.to_i == 0x001c0000
      # XXX hack
      # This waits until the system is in reset
      while mdmap.read_ap(0) & 0b1110 != 0b0010
        mdmap.write_ap(4, 0b11100)
      end
      # Now take system out of reset, but keep core in reset
      while mdmap.read_ap(0) & 0b1110 != 0b1010
        mdmap.write_ap(4, 0b10100)
      end
    end

    256.times do |apsel|
      ap = AP.probe(@dp, apsel)
      if ap
        Log :ap, 1, "found AP #{apsel}, #{ap.IDR}, mem: #{ap.mem?}"
        @ap << ap
        if !@dap && ap.mem?
          @dap = ap
        end
      else
        Log :ap, 1, "no AP on #{apsel}, stopping probe"
        break
      end
    end
    @ap
  end
end


if $0 == __FILE__
  p = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  p.probe
  require 'pp'
  pp p.ap[0].devs
end
