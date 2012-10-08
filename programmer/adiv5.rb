require 'adiv5-swd'
require 'log'

class Adiv5
  module DP
    ABORT = 0
    CTRLSTAT = 4

    CDBGRSTREQ = 1 << 26
    CDBGRSTACK = 1 << 27
    CDBGPWRUPREQ = 1 << 28
    CDBGPWRUPACK = 1 << 29
    CSYSPWRUPREQ = 1 << 30
    CSYSPWRUPACK = 1 << 31

    SELECT = 8

    def self.APSEL(idx)
      idx << 24
    end
    def self.APBANKSEL(idx)
      idx << 4
    end

    RDBUF = 12
  end

  class AP
    IDR = 0xfc

    IDR_REVISION_SHIFT = 28
    IDR_REVISION_MASK = 0xf << IDR_REVISION_SHIFT
    IDR_JEP106_SHIFT = 17
    IDR_JEP106_MASK = 0x7ff << IDR_JEP106_SHIFT
    IDR_CLASS_MASK = 1 << 16
    IDR_APID_MASK = 0xff
    IDR_VARIANT_SHIFT = 4
    IDR_VARIANT_MASK = 0xf << IDR_VARIANT_SHIFT
    IDR_TYPE_MASK = 0xf

    def self.probe(dp, apsel)
      ap = new(dp, apsel)
      return nil if not ap.id

      if ap.id[:class] == :mem
        ap = MemAP.new(dp, apsel)
      end

      ap
    rescue RuntimeError
      nil
    end


    attr_reader :id

    def initialize(dp, apsel)
      @dp, @apsel = dp, apsel

      idr = read_ap(IDR)
      if idr != 0
        @id = {
          :idr => idr,
          :revision => (idr & IDR_REVISION_MASK) >> IDR_REVISION_SHIFT,
          :jep106 => (idr & IDR_JEP106_MASK) >> IDR_JEP106_SHIFT,
          :class => (idr & IDR_CLASS_MASK) != 0 ? :mem : nil,
          :apid => idr & IDR_APID_MASK,
          :variant => (idr & IDR_VARIANT_MASK) >> IDR_VARIANT_SHIFT,
          :type => idr & IDR_TYPE_MASK
        }.freeze
      end
    end

    def mem?
      false
    end

    def read_ap(addr, opt={})
      @dp.read(@apsel, addr, opt)
    end

    def write_ap(addr, val)
      @dp.write(@apsel, addr, val)
    end
  end


  class MemAP < AP
    CSW = 0x00
    CSW_SIZE_MASK = 0b111
    CSW_SIZE_32 = 0b010
    CSW_ADDRINC_SHIFT = 4
    CSW_ADDRINC_MASK = 0b11 << CSW_ADDRINC_SHIFT
    def CSW_ADDRINC(mode)
      case mode
      when :off
        0
      when :single
        0b01 << CSW_ADDRINC_SHIFT
      end
    end
    CSW_MODE_SHIFT = 8
    CSW_MODE_MASK = 0b1111 << CSW_MODE_SHIFT

    TAR = 0x04
    DRW = 0x0c
    class BD
      def self.[](idx)
        0x10 + idx * 4
      end
    end

    CFG = 0xf4
    CFG_BIGENDIAN_MASK = 1

    BASE = 0xf8
    BASE_BASEADDR_SHIFT = 12
    BASE_BASEADDR_MASK = 0xfffff << BASE_BASEADDR_SHIFT
    BASE_FORMAT = 2
    BASE_PRESENT = 1


    class DebugDevice
      # standard mem registers

      PERIPHERAL4 = 0xfd0
      PERIPHERAL0 = 0xfe0
      PERIPHERAL_4KBCOUNT_SHIFT = 36
      PERIPHERAL_4KBCOUNT_MASK = 0xf << PERIPHERAL_4KBCOUNT_SHIFT
      PERIPHERAL_JEP106CC_SHIFT = 32
      PERIPHERAL_JEP106CC_MASK = 0xf << PERIPHERAL_JEP106CC_SHIFT
      PERIPHERAL_REVAND_SHIFT = 28
      PERIPHERAL_REVAND_MASK = 0xf << PERIPHERAL_REVAND_SHIFT
      PERIPHERAL_CUSTMOD_SHIFT = 24
      PERIPHERAL_CUSTMOD_MASK = 0xf << PERIPHERAL_CUSTMOD_SHIFT
      PERIPHERAL_REVISION_SHIFT = 20
      PERIPHERAL_REVISION_MASK = 0xf << PERIPHERAL_REVISION_SHIFT
      PERIPHERAL_JEP106USED = 1 << 19
      PERIPHERAL_JEP106_SHIFT = 12
      PERIPHERAL_JEP106_MASK = 0x3f << PERIPHERAL_JEP106_SHIFT
      PERIPHERAL_PARTNO_MASK = 0xfff

      COMPONENT0 = 0xff0
      COMPONENT_PREAMBLE_MASK = 0xffff0fff
      COMPONENT_PREAMBLE = 0xb105000d
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
          dev = ROMTable.new(memap, base)
        end
      end

      attr_reader :component_class, :device_size

      def initialize(memap, base)
        @mem, @base = memap, base

        Log :ap, 1, '%s at %08x' % [self.class, @base]

        comps = @mem.read(@base + COMPONENT0, :count => 4)
        comp = 0
        comps.each_with_index do |c, i|
          comp |= (c & 0xff) << (i * 8)
        end
        if comp & COMPONENT_PREAMBLE_MASK == COMPONENT_PREAMBLE
          @component_class = COMPONENT_CLASSES[(comp & COMPONENT_CLASS_MASK) >> COMPONENT_CLASS_SHIFT]
          Log :ap, 2, 'device component class:', @component_class
        end

        periphs = @mem.read(@base + PERIPHERAL4, :count => 8)
        periphs = periphs[4..7] + periphs[0..3]
        periph = 0
        periphs.each_with_index do |c, i|
          periph |= (c & 0xff) << (i * 8)
        end
        @device_size = 4 * 1024 << ((periph & PERIPHERAL_4KBCOUNT_MASK) >> PERIPHERAL_4KBCOUNT_SHIFT)
        Log :ap, 3, 'device perhiph id: %016x' % periph
        Log :ap, 2, 'device size: %d' % @device_size
      end
    end

    class ROMTable < DebugDevice
      MEMTYPE = 0xfcc
      MEMTYPE_SYSMEM = 1

      ENTRY_ADDROFFS_SHIFT = 12
      ENTRY_ADDROFFS_MASK = 0xfffff << ENTRY_ADDROFFS_SHIFT
      ENTRY_FORMAT = 1 << 1
      ENTRY_PRESENT = 1

      def initialize(*args)
        super(*args)

        memtype = @mem.read(@base + MEMTYPE)
        @sysmem = memtype & MEMTYPE_SYSMEM != 0

        format = @mem.read(@base)
        if format & ENTRY_FORMAT != 0
          @format = 32
        else
          @format = 8
        end

        scan
      end

      def scan
        @devs = []
        addr = 0
        while true
          entry = read_entry(addr)
          break if entry == 0
          addr += 4
          Log :ap, 2, 'rom table entry %08x' % entry
          next if entry & ENTRY_PRESENT == 0
          devbase = (@base + (entry & ENTRY_ADDROFFS_MASK)) & 0xffffffff
          dev = DebugDevice.probe(@mem, devbase)
          devs << dev if dev
        end
      end

      def read_entry(addr)
        if @format == 32
          @mem.read(@base + addr)
        else
          val = 0
          4.times do |i|
            val |= @mem.read(@base + addr + i * 4) << (i * 8)
          end
          val
        end
      end
    end


    def mem?
      true
    end

    def initialize(*args)
      super(*args)

      Log :ap, 1, "initializing memap #@apsel"
      csw = read_ap(CSW)
      csw = (csw & ~CSW_SIZE_MASK) | CSW_SIZE_32
      csw = (csw & ~CSW_ADDRINC_MASK) | CSW_ADDRINC(:single)
      csw = (csw & ~CSW_MODE_MASK)
      write_ap(CSW, csw)
      @endian = (read_ap(CFG) & CFG_BIGENDIAN_MASK) != 0 ? :big : :little
      base = read_ap(BASE)

      if base != 0xffffffff && @base & BASE_FORMAT != 0 && @base & BASE_PRESENT != 0
        base &= BASE_BASEADDR_MASK
        @dev = DebugDevice.probe(self, base)
      end
    end

    def read(addr, opt={})
      write_ap(TAR, addr)
      read_ap(DRW, opt)
    end

    def write(addr, val)
      write_ap(TAR, addr)
      write_ap(DRW, val)
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

    Log :ap, 1, "all systems up"
  end

  def write(apsel, addr, val)
    select apsel, addr
    @dp.write(:ap, addr & 0xc, val)
  end

  def read(apsel, addr, opt={})
    select apsel, addr
    @dp.read(:ap, addr & 0xc, opt)
  end

  def select(apsel, addr)
    select = DP::APSEL(apsel) | DP::APBANKSEL(addr >> 4)
    return if @last_select == select
    @dp.write(:dp, DP::SELECT, select)
    @last_select = select
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
    mdmap = AP.probe(self, 1)
    if mdmap && mdmap.id[:idr] == 0x001c0000
      # XXX hack
      while mdmap.read_ap(0) & 0b110 != 0b010
        mdmap.write_ap(4, 0b11100)
      end
    end

    256.times do |apsel|
      ap = AP.probe(self, apsel)
      if ap
        Log :ap, 1, "found AP #{apsel}", ap.id, "mem: #{ap.mem?}"
      else
        Log :ap, 1, "no AP on #{apsel}, stopping probe"
        break
      end
    end
    nil
  end
end


if $0 == __FILE__
  p = Adiv5.new(FtdiSwd, :vid => Integer(ARGV[0]), :pid => Integer(ARGV[1]), :debug => true)
  p.probe
end
