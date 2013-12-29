require 'adiv5'
require 'armv7'
require 'register'

class KinetisBase < ARMv7
  def initialize(adiv5)
    super(adiv5)
    @mdmap = adiv5.ap(1)
    if !@mdmap || @mdmap.IDR.to_i != 0x001c0000
      raise RuntimeError, "not a Kinetis device"
    end
  end

  def magic_halt
    # Kinetis hack: hold system & core in reset, so that flash and
    # security will have a chance to init, and we have a chance to
    # access the system.  If we don't hold the system in reset, it
    # might loop resetting itself.  While the system resets, it will
    # block debugger access until it has read the security bits.  If
    # the core loops in reset (e.g. because of empty flash), we will
    # get kicked in the nuts regularly.  Holding the system & core in
    # reset prevents this.
    # XXX hack

    # We might have a secure device
    if (status = @mdmap.read_ap(0)) & 0b100 != 0
      Log(:kinetis, 1){ "chip is secure, need mass erase to attach" }
    end

    Log(:kinetis, 1){ "holding system in reset" }
    # This waits until the system is in reset
    while @mdmap.read_ap(0) & 0b1110 != 0b0010
      @mdmap.write_ap(4, 0b11100)
    end
    Log(:kinetis, 1){ "system out of reset, core in reset" }
    # Now take system out of reset, but keep core in reset
    while @mdmap.read_ap(0) & 0b1110 != 0b1010
      @mdmap.write_ap(4, 0b10000)
    end

    self.enable_debug!
    self.catch_vector!(:CORERESET)

    # Now release the core from reset
    Log(:kinetis, 1){ "releasing core from reset" }
    @mdmap.write_ap(4, 0)

    # self.halt_core!
    # self.catch_vector!(:CORERESET, false)
    # self.disable_debug!
  end

  def mass_erase
    status = @mdmap.read_ap(0)
    if status & 0b100 != 0 && status & 0b100000 == 0
      raise RuntimeError, "chip secure and mass erase disabled"
    end
    Log(:kinetis, 1){ "please keep chip in reset.  retry if necessary." }
    # trigger mass erase and wait for acknowledge to come on
    @mdmap.write_ap(4, 0b1)
    while @mdmap.read_ap(0) & 0b1 != 0b1
      @mdmap.write_ap(4, 0b1)
    end
    # wait for progress to finish
    Log(:kinetis, 1){ "wait for erase to finish" }
    while @mdmap.read_ap(4) & 0b1 != 0
      sleep 0.1
    end
    # now we should be erased and unsecure
    Log(:kinetis, 1){ "you can release reset now." }
  end
end

class Kinetis < KinetisBase
  Log[:kinetis] ||= 1

  class FlashSecureError < RuntimeError
  end

  class FTFL
    include Peripheral

    default_address 0x40020000

    register :FSTAT, 0x00 do
      # Flash Status Register, 0x00
      bool :CCIF, [0x00, 7], :desc => "Command Complete Interrupt"
      bool :RDCOLERR, [0x00, 6], :desc => "Read Collision Error"
      bool :ACCERR, [0x00, 5], :desc => "Access Error"
      bool :FPVIOL, [0x00, 4], :desc => "Protection Violation"
      bool :MGSTAT0, [0x00, 0], :desc => "Completion Status"

      # Flash Configuration Register, 0x01
      bool :CIE, [0x01, 7], :desc => "Command Complete Interrupt"
      bool :RDCOLLIE, [0x01, 6], :desc => "Read Collision Error Interrupt"
      bool :ERSAREQ, [0x01, 5], :desc => "Erase All Request"
      bool :ERSSUSP, [0x01, 4], :desc => "Erase Suspend"
      bool :PFLSH, [0x01, 2], :desc => "Flash memory configuration"
      bool :RAMRDY, [0x01, 1], :desc => "RAM Ready"
      bool :EEERDY, [0x01, 0], :desc => "EEPROM read ready"

      # Flash Security Register, 0x02
      unsigned :KEYEN, [0x02, 7..6], :desc => "Backdoor Key Security"
      unsigned :MEEN, [0x02, 5..4], :desc => "Mass Erase Enable Bits"
      unsigned :FSLACC, [0x02, 3..2], :desc => "Failure Analysis Access Code"
      unsigned :SEC, [0x02, 1..0], :desc => "Flash Security"

      # Flash Option Register, 0x03
      # Defined in 6.3.3
      enum :NMI_DIS, [0x03, 2], {
        :disabled => 0,
        :enabled => 1
      }
      enum :EZPORT_DIS, [0x03, 1], {
        :disabled => 0,
        :enabled => 1
      }
      enum :LPBOOT, [0x03, 0], {
        :slow => 0,
        :fast => 1
      }
    end

    unsigned :FCCOB, 0x04, :vector => 3

    register :FCCOB_Program_Section, 0x04 do
      unsigned :addr, [0, 23..0]
      unsigned :fcmd, [3, 7..0]
      unsigned :num_words, [6, 15..0]

      def initialize(addr, num_words)
        super()
        self.fcmd = 0x0b
        self.addr = addr
        self.num_words = num_words
      end
    end

    register :FCCOB_Erase_Sector, 0x04 do
      unsigned :addr, [0, 23..0]
      unsigned :fcmd, [3, 7..0]

      def initialize(addr)
        super()
        self.fcmd = 0x09
        self.addr = addr
      end
    end

    register :FCCOB_Set_FlexRAM, 0x04 do
      enum :function, [2, 7..0], {
        :eeprom => 0x00,
        :ram => 0xff
      }
      unsigned :fcmd, [3, 7..0]

      def initialize(mode)
        super()
        self.fcmd = 0x81
        self.function = mode
      end
    end

    register :FPROT, 0x10 do
      unsigned :FPROT3, 7..0
      unsigned :FPROT2, 15..8
      unsigned :FPROT1, 23..16
      unsigned :FPROT0, 31..24
    end

    register :FEPROT, 0x14 do
      unsigned :EPROT, 23..16
      unsigned :DPROT, 31..24
    end

    def cmd(cmd_fccob)
      self.FCCOB = cmd_fccob

      # clear all errors and start command
      self.FSTAT.transact do |fstat|
        fstat.zero!
        fstat.CCIF = true
        fstat.RDCOLERR = true
        fstat.ACCERR = true
        fstat.FPVIOL = true
      end
      while !self.FSTAT.CCIF
        Log(:kinetis, 3){ "waiting for flash operation completion" }
        sleep 0.01
      end
      raise RuntimeError, "error in flash execution" if self.FSTAT.MGSTAT0
    end
  end

  class FlexRAM
    include Peripheral

    default_address 0x14000000
  end

  class Flash_Config_Field
    include Peripheral

    default_address 0x400

    # XXX would be nice to use the same register from above
    register :flash_config, 0x00 do
      unsigned :KEY0, [0x00, 31..0], :endian => :big
      unsigned :KEY1, [0x04, 31..0], :endian => :big
      unsigned :FPROT, [0x08, 31..0], :endian => :big
      enum :KEYEN, [0x0c, 7..6], {
        :disabled0 => 0b00,
        :disabled => 0b01,
        :enabled => 0b10,
        :disabled3 => 0b11
      }
      enum :MEEN, [0x0c, 5..4], {
        :enabled0 => 0b00,
        :enabled1 => 0b01,
        :disabled => 0b10,
        :enabled => 0b11
      }
      enum :FSLACC, [0x0c, 3..2], {
        :granted0 => 0b00,
        :denied1 => 0b01,
        :denied => 0b10,
        :granted => 0b11
      }
      enum :SEC, [0x0c, 1..0], {
        :secure0 => 0b00,
        :secure1 => 0b01,
        :unsecure => 0b10,
        :secure => 0b11
      }
      unsigned :FOPT, [0x0d, 7..0]
      unsigned :FEPROT, [0x0e, 7..0]
      unsigned :FDPROT, [0x0f, 7..0]
    end
  end

  class SIM
    include Peripheral

    default_address 0x40047000

    register :SOPT, 0x00 do
      bool :USBREGEN, 31
      bool :USBSSTBY, 30
      bool :USBVSTBY, 29
      enum :OSC32KSEL, 19..18, {
        :OSC32KCKL => 0b00,
        :RTC32K => 0b10,
        :LPO1K => 0b11
      }
      enum :RAMSIZE, 15..12, {
        8192 => 0b0001,
        16384 => 0b0011
      }
    end

    register :SOPT1CFG, 0x04 do
      bool :USSWE, 26
      bool :UVSWE, 25
      bool :URWE, 24
    end

    register :SOPT2, 0x1004 do
      enum :USBSRC, 18, {
        :USB_CLKIN => 0,
        :MCGPLLFLLCLK => 1
      }
      enum :PLLFLLSEL, 16, {
        :MCGFLLCLK => 0,
        :MCGPLLCLK => 1
      }
      enum :TRACECLKSEL, 12, {
        :MCGOUTCLK => 0,
        :CORECLK => 1
      }
      enum :PTD7PAD, 11, {
        :single => 0,
        :double => 1
      }
      enum :CLKOUTSEL, 7..5, {
        :FLASH => 0b010,
        :LPO => 0b011,
        :MCGIRCLK => 0b100,
        :RTC32K => 0b101,
        :OSCERCLK0 => 0b110
      }
      enum :RTCCLKOUTSEL, 4, {
        :RTC1Hz => 0,
        :RTC32K => 1
      }
    end

    register :SOPT4, 0x100c do
      enum :FTM0TRG0SRC, 28, {
        :HSCMP0 => 0,
        :FTM1MATCH => 1
      }
      enum :FTM1CLKSEL, 25, {
        :FTM_CLK0 => 0,
        :FTM_CLK1 => 1
      }
      enum :FTM0CLKSEL, 24, {
        :FTM_CLK0 => 0,
        :FTM_CLK1 => 1
      }
      enum :FTM1CH0SRC, 19..18, {
        :FTM_CH0 => 0b00,
        :CMP0 => 0b01,
        :CMP1 => 0b10,
        :USB_SOF => 0b11
      }
      enum :FTM1FLT0, 4, {
        :FTM1_FLT0 => 0,
        :CMP0 => 1
      }
      enum :FTM0FLT1, 1, {
        :FTM0_FLT1 => 0,
        :CMP1 => 1
      }
      enum :FTM0FLT0, 0, {
        :FTM0_FLT0 => 0,
        :CMP0 => 1
      }
    end

    register :SOPT5, 0x1010 do
      enum :UART1RXSRC, 7..6, {
        :UART1_RX => 0b00,
        :CMP0 => 0b01,
        :CMP1 => 0b10
      }
      enum :UART1TXSRC, 4, {
        :UART1_TX => 0,
        :UART1_TX_FTM1_CH0 => 1
      }
      enum :UART0RXSRC, 3..2, {
        :UART0_RX => 0b00,
        :CMP0 => 0b01,
        :CMP1 => 0b10
      }
      enum :UART0TXSRC, 4, {
        :UART0_TX => 0,
        :UART0_TX_FTM1_CH0 => 1
      }
    end

    register :SOPT7, 0x1018 do
      enum :ADC0ALTTRGEN, 7, {
        :PDB => 0,
        :alternate => 1
      }
      enum :ADC0PRETRGSEL, 4, {
        :A => 0,
        :B => 1
      }
      enum :ADC0TRGSEL, 3..0, {
        :PDB0_EXTRG => 0b0000,
        :HSCMP0 => 0b0001,
        :HSCMP1 => 0b0010,
        :PIT0 => 0b0100,
        :PIT1 => 0b0101,
        :PIT2 => 0b0110,
        :PIT3 => 0b0111,
        :FTM0 => 0b1000,
        :FTM1 => 0b1001,
        :RTC_alarm => 0b1100,
        :RTC_seconds => 0b1101,
        :LPT => 0b1110
      }
    end

    register :SDID, 0x1024 do
      unsigned :REVID, 15..12
      enum :FAMID, 6..4, {
        :K10 => 0b000,
        :K20 => 0b001
      }
      enum :PINID, 3..0, {
        32 => 0b0010,
        48 => 0b0100,
        64 => 0b0101
      }
    end

    register :SCGC4, 0x1034 do
      bool :VREF, 20
      bool :CMP, 19
      bool :USBOTG, 18
      bool :UART2, 12
      bool :UART1, 11
      bool :UART0, 10
      bool :I2C0, 6
      bool :CMT, 2
      bool :EWM, 1
    end

    register :SCGC5, 0x1038 do
      bool :PORTE, 13
      bool :PORTD, 12
      bool :PORTC, 11
      bool :PORTB, 10
      bool :PORTA, 9
      bool :TSI, 5
      bool :LPTIMER, 0
    end

    register :SCGC6, 0x103c do
      bool :RTC, 29
      bool :ADC0, 27
      bool :FTM1, 25
      bool :FTM0, 24
      bool :PIT, 23
      bool :PDB, 22
      bool :USBDCD, 21
      bool :CRC, 18
      bool :I2S, 15
      bool :SPI0, 12
      bool :DMAMUX, 1
      bool :FTFL, 0
    end

    register :SCGC7, 0x1040 do
      bool :DMA, 1
    end

    register :CLKDIV1, 0x1044 do
      unsigned :OUTDIV1, 31..28
      unsigned :OUTDIV2, 27..24
      unsigned :OUTDIV4, 19..16
    end

    register :CLKDIV2, 0x1048 do
      unsigned :USBDIV, 3..1
      unsigned :USBFRAC, 0
    end

    register :FCFG1, 0x104c do
      enum :NVMSIZE, 31..28, {
        0 => 0b0000,
        32768 => 0b0011
      }
      enum :PFSIZE, 27..24, {
        32768 => 0b0011,
        65536 => 0b0101,
        0x20000 => 0b0111
      }
      enum :EESIZE, 19..16, {
        2048 => 0b0011,
        1024 => 0b0100,
        512 => 0b0101,
        256 => 0b0110,
        128 => 0b0111,
        64 => 0b1000,
        32 => 0b1001,
        0 => 0b1111
      }
      unsigned :DEPART, 11..8
      bool :FLASHDOZE, 1
      bool :FLASHDIS, 0
    end

    register :FCFG2, 0x1050 do
      unsigned :MAXADDR0, 30..24
      enum :PFLSH, 23, {
        :FlexNVM => 0,
        :program => 1
      }
      unsigned :MAXADDR1, 22..16
    end

    unsigned :UID, 0x1054, :vector => 4
  end

  def initialize(adiv5, magic_halt=false)
    super(adiv5)
    begin
      self.magic_halt if magic_halt
      self.probe!
    rescue
      # If we were unsuccessful, maybe we need to do kinetis reset magic
      if !magic_halt
        Log(:kinetis, 1){ "trouble initializing, retrying with halt" }
        magic_halt = true
        retry
      else
        raise
      end
    end

    @ftfl = Kinetis::FTFL.new(@dap)
    @flexram = Kinetis::FlexRAM.new(@dap)
    @sim = Kinetis::SIM.new(@dap)
    @sector_size = 1024
  end

  def program_sector(addr, data)
    if String === data
      data = (data + "\0"*3).unpack('L*')
    end

    if data.size != @sector_size / 4 || (addr & (@sector_size - 1) != 0)
      raise RuntimeError, "invalid data size or alignment"
    end

    # Flash config field address
    if addr == 0x400
      fconfig = Flash_Config_Field.new(Peripheral::CachingProxy.new(data), 0).flash_config.dup
      if fconfig.SEC != :unsecure
        raise RuntimeError, "flash protection bits will brick device"
      end
    end

    if !@ftfl.FSTAT.RAMRDY
      # set FlexRAM to RAM
      @ftfl.cmd(FTFL::FCCOB_Set_FlexRAM.new(:ram))
    end

    @ftfl.cmd(FTFL::FCCOB_Erase_Sector.new(addr))
    @flexram.write(0, data)
    @ftfl.cmd(FTFL::FCCOB_Program_Section.new(addr, data.size))
  end

  def program(addr, data)
    if addr & (@sector_size - 1) != 0
      raise RuntimeError, "program needs to start on sector boundary"
    end

    if !self.core_halted?
      raise RuntimeError, "can only program flash when core is halted"
    end

    # pad data
    if data.bytesize % @sector_size != 0
      data += ([0xff] * (@sector_size - data.bytesize % @sector_size)).pack('c*')
    end

    datapos = 0
    while datapos < data.bytesize
      sectaddr = addr + datapos
      sector = data.byteslice(datapos, @sector_size)
      yield [sectaddr, datapos, data.bytesize] if block_given?
      program_sector(sectaddr, sector)
      datapos += @sector_size
    end
  end

  def mmap_ranges
    ramsize = @sim.SOPT.RAMSIZE
    flashsize = @sim.FCFG1.PFSIZE
    super +
      [
       {:type => :flash, :start => 0, :length => flashsize, :blocksize => @sector_size},
       {:type => :ram, :start => 0x20000000 - ramsize/2, :length => ramsize}
      ]
  end
end

if $0 == __FILE__
  require 'backend-driver'
  adiv5 = Adiv5.new(BackendDriver.from_string(ARGV[0]))
  k = Kinetis.new(adiv5)
  # r = k.program_sector(0x800, "\xa5"*1024)
  # puts Log.hexary(adiv5.dap.read(0x800, :count => 1024/4))
end
