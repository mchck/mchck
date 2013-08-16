require 'register'

class ARMv7::DWT
  include Peripheral

  register :DWT_CTRL, 0xe0001000 do
    unsigned :NUMCOMP, 31..28
    bool :NOTRCPKT, 27
    bool :NOEXTTRIG, 26
    bool :NOCYNCCNT, 25
    bool :NOPRFCNT, 24
    bool :CYCEVTENA, 22
    bool :FOLDEVTENA, 21
    bool :LSUEVTENA, 20
    bool :SLEEPEVTENA, 19
    bool :EXCEVTENA, 18
    bool :CPIEEVTENA, 17
    bool :EXCTRCENA, 16
    bool :PCSAMPLENA, 12
    unsigned :SYNCTAP, 11..10
    enum :CYCTAP, 9, {
      :CYCCNT6 => 0,
      :CYCCNT10 => 1
    }
    unsigned :POSTCNT, 8..5
    unsigned :POSTPRESET, 4..1
    bool :CYCCNTENA, 0
  end

  unsigned :DWT_CYCCNT, 0xe0001004
  unsigned :DWT_CPICNT, 0xe0001008
  unsigned :DWT_EXCCNT, 0xe000100c
  unsigned :DWT_SLEEPCNT, 0xe0001010
  unsigned :DWT_LSUCNT, 0xe0001014
  unsigned :DWT_FOLDCNT, 0xe0001018
  unsigned :DWT_PCSR, 0xe000101c

  unsigned :DWT_COMP, 0xe0001020, :vector => 16, :stride => 16
  unsigned :DWT_MASK, 0xe0001024, :vector => 16, :stride => 16
  register :DWT_FUNCTION, 0xe0001028, :vector => 16, :stride => 16 do
    bool :MATCHED, 24
    unsigned :DATAVADDR1, 19..16
    unsigned :DATAVADDR0, 15..12
    enum :DATAVSIZE, 11..10, {
      :byte => 0b00,
      :half => 0b01,
      :word => 0b10
    }
    bool :LNK1ENA, 9
    bool :DATAVMATCH, 8
    bool :CYCMATCH, 7
    bool :EMITRANGE, 5
    enum :FUNCTION, 3..0, {
      :disabled => 0b0000,
      :sample_pc => 0b0001,
      :sample_daddr => 0b0001,
      :sample_data => 0b0010,
      :sample_pc_data => 0b0011,
      :watch_pc => 0b0100,
      :watch_read => 0b0101,
      :watch_write => 0b0110,
      :watch_access => 0b0111,
      :cmpmatch_pc => 0b1000,
      :cmpmatch_read => 0b1001,
      :cmpmatch_write => 0b1010,
      :cmpmatch_access => 0b1011,
      :sample_data_read => 0b1100,
      :sample_data_write => 0b1101,
      :sample_pc_data_read => 0b1110,
      :sample_pc_data_write => 0b1111
    }
  end
end
