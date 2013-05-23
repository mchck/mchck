require 'register'

class ARMv7::FPB
  include Peripheral

  register :FP_CTRL, 0xe0002000 do
    unsigned :NUM_CODE2, 14..12
    unsigned :NUM_LIT, 11..8
    unsigned :NUM_CODE1, 7..4
    enum :KEY, 1, {
      :key => 1
    }
    bool :ENABLE, 0

    def NUM_CODE
      v = self.dup
      (v.NUM_CODE2 << self.class.get_field(:NUM_CODE1).width) | v.NUM_CODE1
    end
  end

  register :FP_REMAP, 0xe0002004 do
    bool :REMAP_SRAM, 29
    unsigned :REMAP, 28..5, :absolute => true
  end

  register :FP_COMP, 0xe0002008, :vector => 142 do
    enum :REPLACE, 31..30, {
      :remap => 0b00,
      :lower => 0b01,
      :upper => 0b10,
      :both => 0b11
    }
    unsigned :COMP, 28..2, :absolute => true
    bool :ENABLE, 0
  end
end
