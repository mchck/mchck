require 'log'
require 'adiv5'
require 'register'

class ARMv7
  class SCS
    include Peripheral

    register :DFSR, 0xe000ed30 do
      bool :EXTERNAL, 4
      bool :VCATCH, 3
      bool :DWTTRAP, 2
      bool :BKPT, 1
      bool :HALTED, 0
    end

    register :DHCSR, 0xe000edf0 do
      unsigned :DBGKEY, 31..16  # W
      bool :S_RESET_ST, 25      # R
      bool :S_RETIRE_ST, 24     # R
      bool :S_LOCKUP, 19        # R
      bool :S_SLEEP, 18         # R
      bool :S_HALT, 17          # R
      bool :S_REGRDY, 16        # R
      bool :C_SNAPSTALL, 5      # R/W
      bool :C_MASKINTS, 3       # R/W
      bool :C_STEP, 2           # R/W
      bool :C_HALT, 1           # R/W
      bool :C_DEBUGEN, 0        # R/W
    end

    register :DCRSR, 0xe000edf4 do
      enum :REGWnR, 16, {
        :read => 0,
        :write => 1
      }
      enum :REGSEL, 4..0, {
        :r0 => 0,
        :r1 => 1,
        :r2 => 2,
        :r3 => 3,
        :r4 => 4,
        :r5 => 5,
        :r6 => 6,
        :r7 => 7,
        :r8 => 8,
        :r9 => 9,
        :r10 => 10,
        :r11 => 11,
        :r12 => 12,
        :sp => 0b01101,
        :lr => 0b01110,
        :pc => 0b01111,
        :xPSR => 0b10000,
        :MSP => 0b10001,
        :PSP => 0b10010,
        :flags => 0b10100
      }
    end

    unsigned :DCRDR, 0xe000edf8

    register :DEMCR, 0xe000edfc do
      bool :TRCENA, 24
      bool :MON_REQ, 19
      bool :MON_STEP, 18
      bool :MON_PEND, 17
      bool :MON_EN, 16
      bool :VC_HARDERR, 10
      bool :VC_INTERR, 9
      bool :VC_BUSERR, 8
      bool :VC_STATERR,7
      bool :VC_CHKERR, 6
      bool :VC_NOCPERR, 5
      bool :VC_MMERR, 4
      bool :VC_CORERESET, 0
    end
  end

  def initialize(adiv5)
    @dap = adiv5.dap
    @scs = SCS.new(@dap)
  end

  def probe!
    @dap.devs.each do |d|
      puts "%s at %08x" % [d, d.base]
      # case d.base
      #   # when 0xe0001000
      #   #   @dwt = DWT.new(@dap)
      #   # when 0xe0002000
      #   #   @fpb = FPB.new(@dap)
      #   # when 0xe0000000
      #   #   @itm = ITM.new(@dap)
      #   # when 0xe0040000
      #   #   @tpiu = TPIU.new(@dap)
      #   # when 0xe0041000
      #   #   @etm = ETM.new(@dap)
      # end
    end
  end

  def enable_debug!
    Log :arm, 1, "enabling debug"
    @scs.DHCSR.transact do |dhcsr|
      dhcsr.zero!
      dhcsr.DBGKEY = 0xa05f
      dhcsr.C_DEBUGEN = true
    end
  end

  def catch_vector!(name)
    Log :arm, 1, "catching vector #{name}"
    vcname = "VC_#{name}"
    @scs.DEMCR.send("#{vcname}=", true)
  end

  def halt_core!
    Log :arm, 1, "waiting for core to halt"
    while !@scs.DHCSR.S_HALT
      @scs.DHCSR.transact do |dhcsr|
        dhcsr.DBGKEY = 0xa05f
        dhcsr.C_HALT = true
      end
    end
  end
end

if $0 == __FILE__
  adiv5 = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  armv7 = ARMv7.new(adiv5)
  armv7.enable_debug!
  armv7.catch_vector!(:CORERESET)
  armv7.halt_core!
end
