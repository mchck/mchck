require 'register'

class ARMv7::SCS
  include Peripheral

  register :AIRCR, 0xe000ed0c do
    enum :VECTKEY, 31..16, {
      :key => 0x05fa
    }
    bool :ENDIANESS, 15
    unsigned :PRIGROUP, 10..8
    bool :SYSRESETREQ, 2
    bool :VECTCLRACTIVE, 1
    bool :VECTRESET, 0
  end

  register :DFSR, 0xe000ed30 do
    bool :EXTERNAL, 4
    bool :VCATCH, 3
    bool :DWTTRAP, 2
    bool :BKPT, 1
    bool :HALTED, 0
  end

  register :DHCSR, 0xe000edf0 do
    enum :DBGKEY, 31..16, {   # W
      :key => 0xa05f
    }
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
      :xpsr => 0b10000,
      :msp => 0b10001,
      :psp => 0b10010,
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
