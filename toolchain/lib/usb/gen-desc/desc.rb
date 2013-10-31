$: << File.dirname(__FILE__)

require 'dsl'

class EndpointDesc < DslItem
  field :direction, :enum => {:in => 1, :out => 0}
  field :type, :enum => {:intr => :USB_EP_INTR, :bulk => :USB_EP_BULK}
  field :wMaxPacketSize
  field :bInterval

  def initialize(name)
    super()
    @name = name
  end

  def renumber!(counts)
    cname = "ep_#@direction".to_sym
    @epnum = counts[cname]
    counts.merge cname => @epnum + 1
  end

  def gen_desc_init
    <<_end_
.#{@name.to_loc_s} = {
	.bLength = sizeof(struct usb_desc_ep_t),
	.bDescriptorType = USB_DESC_EP,
	.ep_num = #@epnum,
	.in = #{@direction.val},
	.type = #{@type.val},
	.wMaxPacketSize = #{@wMaxPacketSize.to_loc_s},
	.bInterval = #{@bInterval.to_loc_s}
},
_end_
  end
end

class InterfaceDesc < DslItem
  attr_accessor :ifacenum

  field :bInterfaceClass
  field :bInterfaceSubClass
  field :bInterfaceProtocol

  block :ep, EndpointDesc, :list => true, :optional => true
  block :alternate, InterfaceDesc, :list => true, :optional => true

  def initialize(name)
    super()
    @name = name
  end

  def renumber!(counts, alternatenum=0)
    @ifacenum = counts[:iface]
    @alternatenum = alternatenum

    nc = counts.dup
    @ep.each do |e|
      nc = e.renumber!(nc)
    end

    @alternate.each_with_index do |a, i|
      ac = a.renumber!(counts, i)
      [:ep_in, :ep_out].each do |e|
        if ac[e] > nc[e]
          nc[e] = ac[e]
        end
      end
    end
    nc
  end

  def gen_defs
    ''
  end

  def gen_desc_init
    v = <<_end_
.#{@name.to_loc_s} = {
	.bLength = sizeof(struct usb_desc_iface_t),
	.bDescriptorType = USB_DESC_IFACE,
	.bInterfaceNumber = #@ifacenum,
	.bAlternateSetting = #@alternatenum,
	.bNumEndpoints = #{@ep.count},
	.bInterfaceClass = #{@bInterfaceClass.to_loc_s},
	.bInterfaceSubClass = #{@bInterfaceSubClass.to_loc_s},
	.bInterfaceProtocol = #{@bInterfaceProtocol.to_loc_s},
	.iInterface = 0
},
_end_
    v + @ep.map{|e| e.gen_desc_init}.join("\n")
  end
end

class FunctionDesc < DslItem
  block :interface, InterfaceDesc, :list => true

  def renumber!(counts)
    @var_name = "usb_function_#{counts[:iface]}"
    @interface.each do |iface|
      counts = iface.renumber!(counts)
      counts[:iface] += 1
    end
    counts
  end

  def get_desc_struct
    "#{self.class::TypeName} #@var_name;"
  end

  def gen_defs
    @interface.map{|i| i.gen_defs}.join("\n")
  end

  def gen_vars
    ''
  end

  def get_function_var
    self.class::FunctionVarName
  end
end

class ConfigDesc < DslItem
  field :remote_wakeup, :default => 0
  field :self_powered, :default => 0
  field :bMaxPower, :default => 100

  field :initfun

  block :function, FunctionDesc, :list => true

  def renumber!(confignum)
    @confignum = confignum
    counts = {:iface => 0, :ep_in => 0, :ep_out => 0}
    @function.each do |f|
      counts = f.renumber!(counts)
    end
    @numinterfaces = counts[:iface]

    @config_name = "usb_config_#@confignum"
    @var_name = "usbd_config_#@confignum"
  end

  def gen_defs
    @function.map{|f| f.gen_defs}.join +
      <<_end_
struct #@config_name {
	struct usb_desc_config_t config;
	#{@function.map{|f| f.get_desc_struct}.join("\n\t")}
};
_end_
  end

  def gen_vars
    @function.map{|f| f.gen_vars}.join("\n") +
      <<_end_

static const struct #@config_name #@config_name = {
	.config = {
		.bLength = sizeof(struct usb_desc_config_t),
		.bDescriptorType = USB_DESC_CONFIG,
		.wTotalLength = sizeof(struct #@config_name),
		.bNumInterfaces = #@numinterfaces,
		.bConfigurationValue = #@confignum,
		.iConfiguration = 0,
		.one = 1,
		.bMaxPower = #{@bMaxPower.to_loc_s}
	},
	#{@function.map{|f| f.gen_desc_init}.join}
};

static const struct usbd_config #@var_name = {
	.init = #{@initfun.to_loc_s},
	.desc = &#@config_name.config,
  .function = {#{@function.map{|f| "&#{f.get_function_var}"}.join(",\n\t")}},
};
_end_
  end

  def get_var
    @var_name
  end
end

class DeviceDesc < DslItem
  field :idVendor
  field :idProduct
  field :bcdDevice, :default => 0.0
  field :iManufacturer
  field :iProduct

  def initialize(name)
    super()
    @name = name
  end

  block :config, ConfigDesc, :list => true

  def renumber!
    @config.each_with_index do |c, i|
      c.renumber!(i + 1)
    end
  end

  def gen_defs
    @config.map{|c| c.gen_defs}.join("\n")
  end

  def gen_vars
    @config.map{|c| c.gen_vars}.join("\n") +
      <<_end_

static const struct usb_desc_dev_t #{@name.to_loc_s}_dev_desc = {
	.bLength = sizeof(struct usb_desc_dev_t),
	.bDescriptorType = USB_DESC_DEV,
	.bcdUSB = { .maj = 2 },
	.bDeviceClass = USB_DEV_CLASS_SEE_IFACE,
	.bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,
	.bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,
	.bMaxPacketSize0 = EP0_BUFSIZE,
	.idVendor = #{@idVendor.to_loc_s},
	.idProduct = #{@idProduct.to_loc_s},
	.bcdDevice = { .raw = 0 },
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = #{@config.length},
};

static const struct usb_desc_string_t * const #{@name.to_loc_s}_str_desc[] = {
	USB_DESC_STRING_LANG_ENUS,
	USB_DESC_STRING(#{@iManufacturer.to_loc_s{|s| "u#{s.inspect}"}}),
	USB_DESC_STRING(#{@iProduct.to_loc_s{|s| "u#{s.inspect}"}}),
	USB_DESC_STRING_SERIALNO,
	NULL
};

struct usbd_device #{@name.to_loc_s} = {
	.dev_desc = &#{@name.to_loc_s}_dev_desc,
	.string_descs = #{@name.to_loc_s}_str_desc,
	.configs = {
		#{@config.map{|c| "&#{c.get_var},"}.join("\n")}
		NULL
	}
};
_end_
  end
end

class DescriptorRoot < DslItem
  block :device, DeviceDesc, :list => true do |d|
    @device ||= []
    @device << d
    d.renumber!
  end

  def self.load(file)
    src = File.read(file)
    self.eval do
      eval src
    end
  end

  def gen
    @device.map{|d| d.gen_defs}.join("\n") +
    @device.map{|d| d.gen_vars}.join("\n")
  end
end


require 'cdc'
require 'dfu'


if $0 == __FILE__
  require 'optparse'

  outname = nil
  OptionParser.new do |opts|
    opts.on("-o", "--output FILE") do |fn|
      outname = fn
    end
  end.parse!

  r = DescriptorRoot.load(ARGV[0])
  if !(warnings = r.warnings).empty?
    $stderr.puts warnings.join("\n")
    exit 1
  end

  if outname
    File.open(outname, 'w') do |f|
      f.puts r.gen
    end
  else
    puts r.gen
  end
end
