class HIDReportElement
  def initialize(type, tag, data)
    @type, @tag, @data = type, tag, data
  end

  def data_str
    if @data.nil?
      "0"
    else
      @data.to_s
    end
  end

  def renumber!(n)
    @number = n
  end

  def gen_desc_def
    "HID_ITEM_TYPE(#{self.data_str}) hid_item_#@number;"
  end

  def gen_desc_init
    typestr = @type.to_s.upcase
    tagstr = @tag.to_s.upcase
    <<_end_
	.hid_item_#@number = {
		/* #{self.inspect} */
		.bSize = HID_ITEM_SIZE(#{self.data_str}),
		.bType = HID_TYPE_#{typestr},
		.bTag = HID_TAG_#{typestr}_#{tagstr},
		#{!@data.nil? && @data != 0 ? ".value = #@data" : ""}
	},
_end_
  end
end

module HIDUsageStrGen
  def usage_str(usage_page, u = nil)
    s = nil
    if usage_page.is_a? Symbol
      s = "HID_USAGE_#{usage_page.to_s.upcase}"
    else
      s = usage_page.to_s
    end
    if u.is_a? Symbol
      s += "_#{u.to_s.upcase}"
    elsif !u.nil?
      s = u.to_s
    end
    s
  end
end

class HIDReportBaseItem < DslItem
  include HIDUsageStrGen

  def initialize(name, *more_names)
    super()

    if more_names.empty? and name.is_a? Array
      @items = [name.first]
      @usages = [0]               # XXX
      @count = name[1]
    elsif !more_names.empty? or !name.is_a? Hash
      # list of names
      names = [name] + more_names
      if !names.all?{|e| e.is_a? Symbol or e.is_a? Numeric}
        @warnings << "XXX not a list of symbols: #{names}"
        return
      end
      @items = names
      @usages = names
      @count = @items.count
    else
      if !more_names.empty?
        @warnings << "XXX not a single hash"
        return
      end

      if name.each_value.all?{|e| e.is_a? Symbol or e.is_a? Numeric}
        # direct mapping
        @items = name.keys
        @usages = name.values
        @count = @items.count
      elsif name.count == 1 and name.values.first.is_a? Range
        # range mapping
        n, r = name.first
        @items = [n]
        @usage_range = r
        @is_ranged = true
        if r.begin.is_a? Numeric and r.end.is_a? Numeric
          @count = r.end - r.begin + 1
        end
      else
        @warnings << "XXX not mapping to values properly"
      end
    end
  end

  field :size
  field :count

  def relative
    if @relative && @relative != 1 << 2
      @warnings << "#{location}: overwriting setting with `relative'"
    end
    @relative = 1 << 2
  end

  def absolute
    location = caller[0]
    if @relative && @relative != 0
      @warnings << "#{location}: overwriting setting with `absolute'"
    end
    @relative = 0
  end

  def main_flags
    (@relative || 0) |
      2 # XXX variable |
#      1 # XXX constant
  end

  def elements(type, usage_page)
    e = [
         HIDReportElement.new(:global, :report_size, @size),
         HIDReportElement.new(:global, :report_count, @count),
        ]
    up = usage_page.to_s.upcase
    if !@usages.nil?
      @usages.each do |u|
        e << HIDReportElement.new(:local, :usage, usage_str(usage_page, u))
      end
    elsif @is_ranged
      e << HIDReportElement.new(:local, :usage_minimum, usage_str(usage_page, @usage_range.begin))
      e << HIDReportElement.new(:local, :usage_maximum, usage_str(usage_page, @usage_range.end))
    end

    e << HIDReportElement.new(:main, type, self.main_flags)

    e
  end

  def gen_report_def
    t = nil
    if @size <= 8
      t = 8
    elsif @size <= 16
      t = 16
    else
      t = 32
    end

    if @is_ranged
      "uint#{t}_t #{@items.first} : #{@size * @count};"
    else
      if @items.size == @count
        @items.map{|i| "uint#{t}_t #{i} : #@size;"}
      elsif @items.size == 1 && @count > 1 && @size == t
        "uint#{t}_t #{@items.first}[#{@count}];"
      end
    end
  end
end

class HIDReportDesc < DslItem
  attr_accessor :type, :report_id

  def initialize(type)
    super()
    @type = type
    @item = []
  end

  field :name
  field :report_id, :optional => true

  block :item, HIDReportBaseItem, :list => true

  def renumber!(type, counts)
    if @report_id.nil?
      # find new report ID
      id = 1
      while counts[:used].include? id or counts[:assigned].include? id
        id += 1
      end
      @report_id = id
    end
    if counts[:used].include? @report_id
      @warnings << "XXX duplicate #{type} report id #@report_id"
    end
    counts[:used] << @report_id
    counts
  end

  def elements
    e = []
    e << HIDReportElement.new(:global, :report_id, @report_id) if @report_id
    @item.each do |i|
      e += i.elements(@type)
    end

    e
  end

  def size
    s = 0
    s += 1 if @report_id
    s += @item.map{|i| i.get_size * i.get_count}.reduce(&:+)
    (s + 7) / 8
  end

  def gen_report_def
    rid = ""
    if @report_id
      rid = "uint8_t report_id;"
    end
    <<_end_
struct #{@name.to_loc_s} {
	#{rid}
	#{@item.map(&:gen_report_def).flatten.join("\n\t")}
};
_end_
  end
end

# These follow the definition of HIDReportDesc, or child_block won't work.
class HIDReportItem < HIDReportBaseItem
  include HIDUsageStrGen

  child_block :item

  field :logical_range, :optional => true
  field :usage_page, :optional => true

  def elements(type)
    e = []
    if @logical_range
      e += [
        HIDReportElement.new(:global, :logical_minimum, @logical_range.begin),
        HIDReportElement.new(:global, :logical_maximum, @logical_range.end)
      ]
    end
    if @usage_page
      e += [HIDReportElement.new(:global, :usage_page, usage_str(@usage_page))]
    end
    e + super(type, @usage_page)
  end
end

class HIDReportArrayItem < HIDReportItem
  child_block :array

  def initialize(name)
    super
  end
end

class HIDReportPadItem < HIDReportBaseItem
  child_block :pad

  @@idx = 0

  def initialize(count_or_align)
    if count_or_align.is_a? Hash
      @align = count_or_align[:align]
    else
      @size = Integer(count_or_align)
      @count = 1
    end
    # XXX fix report_size/count output
    @id = "_pad#{@@idx}".to_sym
    @@idx += 1
    super(@id)
    @usages = nil               # pad items do not have a usage
  end

  def elements(type)
    super(type, nil)
  end

  # def gen_report_def
  #   "XXX pad #@count * #@size"
  # end
end

class HIDReportCollection < DslItem
  include HIDUsageStrGen

  def initialize(usage_page, usage)
    super()
    @usage_page, @usage = usage_page, usage
  end

  block :report, HIDReportDesc, :list => true

  def report_ids(type)
    @report.select{|r| r.type == type}.map(&:get_report_id)
  end

  def renumber_reports!(type, counts)
    @report.each do |r|
      counts = r.renumber!(type, counts)
    end

    counts
  end

  def elements
    e = [
         HIDReportElement.new(:global, :usage_page, usage_str(@usage_page)),
         HIDReportElement.new(:local, :usage, usage_str(@usage_page, @usage)),
         HIDReportElement.new(:main, :collection, :HID_COLLECTION_APPLICATION)
        ]
    @report.each do |r|
      e += r.elements
    end

    e << HIDReportElement.new(:main, :end_collection, nil)

    e
  end

  def gen_report_defs
    @report.map(&:gen_report_def).join("\n")
    # XXX define enum for report ids
  end
end

class HIDDesc < FunctionDesc
  TypeName = "struct hid_function_desc"
  #FunctionVarName = "hid_function"

  child_block :hid

  @@hid_id = 0

  def initialize(name)
    @name = name
    super()

    init_func :hid_init
    control_func :hid_handle_control

    # generate unique id for hid struct
    @hid_id = @@hid_id
    @@hid_id += 1
  end

  block :collection, HIDReportCollection, :list => true
  field :boot, :optional => true

  METHODS = %w{get_descriptor get_report set_report get_idle set_idle get_protocol set_protocol}.map(&:to_sym)

  METHODS.each do |m|
    field m, :optional => true
  end

  def report_max_size
    @collection.map{|c| c.get_report.select{|r| r.type == :input}.map{|r| r.size}}.flatten.max
  end

  def renumber!(function_counts)
    [:input, :output, :feature].each do |type|
      ids = []
      @collection.each do |c|
        ids += c.report_ids(type)
      end

      if !ids.compact.empty?
        counts = {:used => [], :assigned => ids.compact}
        @collection.each do |c|
          counts = c.renumber_reports!(type, counts)
        end
      end
    end

    @elements = @collection.map(&:elements).flatten
    @elements.each_with_index do |e, i|
      e.renumber!(i)
    end

    hid = self

    interface(:iface) {
      bInterfaceClass :USB_DEV_CLASS_HID
      bInterfaceSubClass 0      # XXX overwrite when boot
      bInterfaceProtocol 0

      ep(:int_in_ep) {
        direction :in
        type :intr
        wMaxPacketSize (hid.report_max_size || 0)
        bInterval 0xff          # XXX configurable?
      }
    }

    super
  end

  def gen_report_defs
    @collection.map(&:gen_report_defs).join
  end

  def gen_defs
    s = <<_end_
struct hid_ctx #{@name.to_loc_s};
_end_
    METHODS.each do |p|
      val = self.instance_variable_get("@#{p}")
      s += "hid_#{p}_t #{val.to_loc_s};\n" if val
    end
    s += self.gen_report_defs
    super() + s
  end

  def gen_vars
    s = <<_end_
struct hid_report_desc_#@hid_id {
	#{@elements.map(&:gen_desc_def).join("\n\t")}
};

static const struct hid_report_desc_#@hid_id hid_report_desc_#@hid_id = {
#{@elements.map(&:gen_desc_init).join}
};

struct hid_ctx #{@name.to_loc_s};

_end_
    s += super
  end

  def gen_func_defs
    <<_end_
	struct hid_function hid_func;
_end_
  end

  def gen_func_init
    s = <<_end_
	.hid_func = {
		.usb_func = #{super},
		.ctx = &#{@name.to_loc_s},
_end_
    METHODS.each do |p|
      val = self.instance_variable_get("@#{p}")
      s += "		.#{p} = #{val.to_loc_s},\n" if val
    end
    s += <<_end_
		.report_desc = &hid_report_desc_#@hid_id,
		.report_desc_size = sizeof(hid_report_desc_#@hid_id),
		.report_max_size = #{self.report_max_size || 0},
	},
_end_
  end

  def get_function_var
    super("hid_func.usb_func")
  end

  def gen_desc_init
    <<_end_
.#@var_name = {
	#{@interface.first.gen_desc_init}
	.hid_desc = {
		.bLength = sizeof(struct hid_desc_t),
		.bDescriptorType = { .raw = USB_HID_REPORT_DESC_TYPE_HID },
		.bcdHID = { .maj = 1, .min = 1 },
		.bCountryCode = 0,
		.bNumDescriptors = 1,
		.bDescriptorType1 = { .raw = USB_HID_REPORT_DESC_TYPE_REPORT },
		.wDescriptorLength1 = sizeof(struct hid_report_desc_#@hid_id),
	},
},
_end_
  end
end
