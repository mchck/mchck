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


    # standard mem registers

    PERIPHERAL4 = 0xfd0
    PERIPHERAL0 = 0xfe0
    COMPONENT0 = 0xff0


    def mem?
      true
    end

    def initialize(*args)
      super(*args)

      Debug "initializing memap #@apsel"
      csw = read_ap(CSW)
      csw = (csw & ~CSW_SIZE_MASK) | CSW_SIZE_32
      csw = (csw & ~CSW_ADDRINC_MASK) | CSW_ADDRINC(:single)
      csw = (csw & ~CSW_MODE_MASK)
      write_ap(CSW, csw)
      @endian = (read_ap(CFG) & CFG_BIGENDIAN_MASK) != 0 ? :big : :little
      @base = read_ap(BASE)

      if @base == 0xffffffff || @base & BASE_FORMAT == 0 || @base & BASE_PRESENT == 0
        @base = nil
      else
        @base &= BASE_BASEADDR_MASK
        comps = read_mem(@base + COMPONENT0, :count => 4)
        comp = 0
        comps.each_with_index do |c, i|
          comp |= (c & 0xff) << (i * 8)
        end

        Debug 'memap component id: %08x' % comp
      end
    end

    def read_mem(addr, opt={})
      write_ap(TAR, addr)
      read_ap(DRW, opt)
    end

    def write_mem(addr, val)
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

    Debug "all systems up"
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
        Debug "found AP #{apsel}", ap.id, "mem: #{ap.mem?}"
      else
        Debug "no AP on #{apsel}, stopping probe"
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
