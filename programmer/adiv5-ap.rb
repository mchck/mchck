require 'register'
require 'log'

class Adiv5
  class UnalignedAccessError < RuntimeError
  end

  class DebugDevice
    # standard mem registers

    include Peripheral

    class BytePeripheral
      include Peripheral

      def initialize(backing)
        @backing = backing
      end

      def get_backing(addr)
        4.times.map do |i|
          @backing.get_backing(get_address(addr + i * 4))
        end.reverse.inject(0){|t,w| (t << 8)|w}
      end
    end

    class PeripheralID < BytePeripheral
      default_address 0xfd0

      register :PERIPHERAL4, 0 do
        unsigned :count_4kb, 7..4
        unsigned :jep106_cc, 3..0
      end

      register :PERIPHERAL0, 4 do
        unsigned :revand, 31..28
        unsigned :custmod, 27..24
        unsigned :revision, 23..20
        bool :jep106_used?, 19
        unsigned :jep106, 18..12
        unsigned :partno, 11..0
      end
    end

    unsigned :COMPONENT, 0xff0, :vector => 4

    COMPONENT_PREAMBLE_MASK = 0xffff0fff
    COMPONENT_PREAMBLES = [0xb105000d, 0xb102000d]
    COMPONENT_CLASS_SHIFT = 12
    COMPONENT_CLASS_MASK = 0xf << COMPONENT_CLASS_SHIFT
    COMPONENT_CLASSES = Hash.new{|h,k| k}.merge({
                                                  0 => :generic_verify,
                                                  1 => :rom,
                                                  9 => :debug,
                                                  11 => :ptb,
                                                  13 => :dess,
                                                  14 => :generic_ip,
                                                  15 => :primecell
                                                }).freeze

    def self.probe(memap, base)
      dev = new(memap, base)

      if dev.component_class == :rom && dev.device_size == 4096
        ROMTable.new(memap, base).devs
      else
        [dev]
      end
    end

    attr_reader :component_class, :device_size
    attr_reader :base

    def initialize(memap, base)
      @base = base
      # for Peripheral
      @backing = memap
      @address = base

      Log(:ap, 1){ '%s at %08x' % [self.class, @base] }

      comp = 0
      @backing.read(get_address(COMPONENT), :count => 4).each_with_index do |v, i|
        comp |= (v & 0xff) << (i * 8)
      end
      if COMPONENT_PREAMBLES.include?(comp & COMPONENT_PREAMBLE_MASK)
        @component_class = COMPONENT_CLASSES[(comp & COMPONENT_CLASS_MASK) >> COMPONENT_CLASS_SHIFT]
        Log(:ap, 2){ "device component class: #@component_class" }
      end

      periph = PeripheralID.new(self)
      @device_size = 4 * 1024 << periph.PERIPHERAL4.count_4kb
      Log(:ap, 2){ 'device size: %d' % @device_size }
    end
  end

  class ROMTable < DebugDevice
    register :MEMTYPE, 0xfcc do
      bool :sysmem?, 1
    end

    register :entries, 0, :vector => (0xf00 / 4) do
      unsigned :addroffs, 31..12, :absolute => true
      bool :format?, 1
      bool :present?, 0
    end

    attr_reader :devs

    def initialize(*args)
      super(*args)

      @sysmem = self.MEMTYPE.sysmem?

      if self.entries[0].format?
        scan
      end
    end

    def scan
      @devs = []
      self.entries.each do |entry|
        break if entry.to_i == 0
        Log(:ap, 2){ 'rom table entry %08x' % entry.to_i }
        next if !entry.present?
        devbase = (@base + entry.addroffs) % (1 << 32)
        devs = DebugDevice.probe(@backing, devbase)
        @devs += devs if devs
      end
    end
  end

  class AP
    include Peripheral

    register :IDR, 0xfc do
      unsigned :revision, 31..28
      unsigned :JEP106, 27..17
      bool :mem?, 16
      unsigned :variant, 7..4
      unsigned :type, 3..0
    end

    def self.probe(dp, apsel)
      ap = new(dp, apsel)
      return nil if not ap.valid?

      if ap.IDR.mem?
        ap = MemAP.new(dp, apsel)
      end

      ap
    rescue StandardError => e
      Log(:ap, 1){ "could not probe AP #{apsel}: #{e}\n#{e.backtrace.join("\n")}" }
      nil
    end


    def initialize(dp, apsel)
      @dp, @apsel = dp, apsel
    end

    def valid?
      self.IDR.to_i != 0
    end

    def mem?
      self.IDR.mem?
    end

    def read_ap(addr, opt={})
      @dp.ap_select(@apsel, addr)
      v = @dp.read(:ap, addr & 0xc, opt)
      Log(:ap, 3){ "%d read %08x < %s" % [@apsel, addr, Log.hexary(v)] }
      v
    end
    alias :get_backing :read_ap

    def write_ap(addr, val)
      Log(:ap, 3){ "%d write %08x = %s" % [@apsel, addr, Log.hexary(val)] }
      @dp.ap_select(@apsel, addr)
      @dp.write(:ap, addr & 0xc, val)
    end
    alias :set_backing :write_ap
  end

  class MemAP < AP
    register :CSW, 0x00 do
      bool :DbgSwEnable?, 31
      unsigned :Prot, 30..24
      bool :SPIDEN?, 23
      unsigned :Mode, 11..8
      bool :TrInProg?, 7
      bool :DeviceEn?, 6
      enum :AddrInc, 5..4, {
        :off => 0b00,
        :single => 0b01,
        :packed => 0b10
      }
      enum :Size, 2..0, {
        :byte => 0b000,
        :halfword => 0b001,
        :word => 0b010
      }
    end

    unsigned :TAR, 0x04
    unsigned :DRW, 0x0c
    unsigned :BD, 0x10, :vector => 4

    register :CFG, 0xf4 do
      enum :endian, 0, {
        :little => 0,
        :big => 1
      }
    end

    register :BASE, 0xf8 do
      unsigned :BASEADDR, 31..12, :absolute => true
      bool :format?, 1
      bool :present?, 0
    end


    def mem?
      true
    end

    attr_reader :endian

    def initialize(*args)
      super(*args)

      Log(:ap, 1){ "initializing memap #@apsel" }
      self.CSW.transact do |csw|
        csw.Size = @last_size = :word
        csw.AddrInc = :single
        csw.Mode = 0
      end
      @endian = self.CFG.endian
    end

    def devs
      return @devs if @devs
      base = self.BASE.dup
      if base.to_i != 0xffffffff && base.format? && base.present?
        @devs = DebugDevice.probe(self, base.BASEADDR)
      end
    end

    def transfer(dir, addr, val_or_opt, opt={})
      if Hash === val_or_opt
        opt.merge! val_or_opt
      end
      size = opt[:size]
      size ||= :word

      align = case size
              when :word
                4
              when :halfword
                2
              when :byte
                1
              end

      raise UnalignedAccessError if addr % align != 0

      count = opt[:count]
      count ||= 1

      transfer_reg = DRW
      if size == :word && count == 1 &&
          @last_TAR && @last_TAR & 0xfffffff0 == addr & 0xfffffff0
        ofs = addr & 0xf
        transfer_reg = BD + ofs
      elsif @last_TAR != addr
        self.TAR = @last_TAR = addr
      end

      if @last_size != size
        Log(:mem, 2){ "switching AP size to #{size}" }
        self.CSW.Size = @last_size = size
      end

      if transfer_reg == DRW
        # clear cache in case we run into an exception and lose track
        @last_TAR = nil
      end
      v = case dir
          when :in
            read_ap(transfer_reg, val_or_opt)
          when :out
            write_ap(transfer_reg, val_or_opt)
          end
      if transfer_reg == DRW
        @last_TAR = addr + count * align
      end

      v
    end

    def read(addr, opt={})
      v = transfer(:in, addr, opt)
      Log(:mem, 1){ "read %08x < %s" % [addr, Log.hexary(v)] }
      v
    end
    alias :[] :read

    def write(addr, val)
      Log(:mem, 1){ "write %08x = %s" % [addr, Log.hexary(val)] }
      transfer(:out, addr, val)
    end
    alias :[]= :write
  end
end
