module Location
  attr_reader :location

  def self.attach(val, location)
    val.singleton_class.instance_exec{include Location}
    val.instance_exec{@location = location}
    val
  rescue Exception => e
    val = LocationWrapper.new(val, location)
    val
  end

  def to_loc_s
    v = if location
          m = @location.match(/^(.*?):(\d+):/)
          "\n#line #{m[2]} #{m[1].inspect}\n"
        else
          ""
        end
    val = self.class == LocationWrapper ? @val : self
    val = yield val if block_given?
    v + val.to_s
  end
end

class LocationWrapper
  include Location

  attr_reader :val

  def initialize(val, location)
    @val = val
    @location = location
  end

  def method_missing(method, *args, &block)
    @val.send(method, *args, &block)
  end

  def respond_to?(m)
    @val.respond_to? m
  end

  def nil?
    @val.nil?
  end

  def to_s
    @val.to_s
  end

  def inspect
    @val.inspect
  end

  def is_a?(klass)
    @val.is_a? klass
  end

  def ==(other)
    other = other.val if other.is_a? LocationWrapper
    @val == other
  end

  def eql?(other)
    other = other.val if other.is_a? LocationWrapper
    @val.eql?(other)
  end
end

class DslItem
  class << self
    def fields
      instance_variable_get(:@fields) || superclass.fields
    end

    def add_field(name, opts)
      begin
        v = self.fields
      rescue NameError
        v = {}
      end
      v = v.merge({name => opts})
      instance_variable_set(:@fields, v)
      define_method "get_#{name}" do
        instance_variable_get("@#{name}")
      end
    end

    def attach_lineno(val, lineno=caller[1])
      Location.attach(val, lineno)
    end

    def field(name, opts = {}, &action)
      opts[:action] = action if action
      add_field(name, opts)
      define_method name do |val|
        set_or_exec(name, val, opts)
      end
    end

    def block(name, klass, opts = {}, &action)
      klass.const_set(:Parent, self)
      opts[:name] = name
      opts[:klass] = klass
      opts[:action] = action if action
      add_field(name, opts)
      field_alias(name, klass)
    end

    def field_alias(name_alias, klass)
      opts = self.fields.values.find{|o| klass.ancestors.include? o[:klass]}
      define_method name_alias do |*args, &block|
        args = args.map do |a|
          DslItem.attach_lineno(a, caller[2])
        end
        val = klass.eval(*args, &block)
        set_or_exec(opts[:name], val, opts)
      end
    end

    def child_block(name)
      superclass::Parent.field_alias(name, self)
    end

    def eval(*args, &block)
      i = self.new(*args)
      i.instance_exec(&block)
      i.post_eval
      i
    end
  end

  def initialize
    @warnings = []
  end

  def post_eval
    self.class.fields.each do |n, o|
      if !instance_variable_defined?("@#{n}")
        instance_variable_set("@#{n}", DslItem.attach_lineno(o[:default] || (o[:list] ? [] : nil), nil))
      end
    end
  end

  def set_or_exec(name, val, opts)
    location = caller[1]
    wrapped_val = DslItem.attach_lineno(val, location)
    if opts[:enum]
      if !opts[:enum].include? val
        @warnings << "#{location}: field `#{name}' value not one of #{opts[:enum].keys.map(&:inspect).join(", ")}"
      else
        wrapped_val.define_singleton_method :val do
          opts[:enum][val]
        end
      end
    end
    if opts[:action]
      instance_exec(val, &opts[:action])
    else
      varname = "@#{name}".to_sym
      if opts[:list]
        if !instance_variable_get(varname)
          instance_variable_set(varname, [])
        end
        instance_variable_get(varname) << wrapped_val
      else
        if instance_variable_defined?(varname)
          @warnings << "#{location}: field `#{name}' redefined"
        end
        instance_variable_set(varname, wrapped_val)
      end
    end
    wrapped_val
  end

  def warnings
    r = @warnings.dup
    self.class.fields.each do |name, opts|
      val = instance_variable_get("@#{name}")
      if !opts.has_key?(:default) && !opts[:optional] && val.nil?
        r << "#@location: required field `#{name}' for class #{self.class} undefined"
      end
      if val.respond_to? :warnings
        r += val.warnings
      elsif val.respond_to? :each
        val.each do |v|
          r += v.warnings if v.respond_to? :warnings
        end
      end
    end
    r
  end
end
