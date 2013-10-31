class LocationWrapper
  attr_reader :location

  def initialize(val, location)
    @val = val
    @location = location
  end

  def method_missing(method, *args, &block)
    @val.send(method, *args, &block)
  end

  def to_s
    @val.to_s
  end

  def inspect
    @val.inspect
  end

  def to_loc_s
    v = if location
          m = @location.match(/^(.*?):(\d+):/)
          "\n#line #{m[2]} #{m[1].inspect}\n"
        else
          ""
        end
    val = self
    val = yield val if block_given?
    v + val.to_s
  end
end

class DslItem
  class << self
    def add_field(name, opts)
      v = {}
      begin
        v = class_variable_get(:@@fields)
      rescue NameError
        class_variable_set(:@@fields, v)
      end
      v[name] = opts
    end

    def attach_lineno(val, lineno=caller[1])
      LocationWrapper.new(val, lineno)
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
      opts = class_variable_get(:@@fields).values.find{|o| klass.ancestors.include? o[:klass]}
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
    self.class.class_variable_get(:@@fields).each do |n, o|
      if !instance_variable_defined?("@#{n}")
        instance_variable_set("@#{n}", DslItem.attach_lineno(o[:default] || (o[:list] ? [] : nil), nil))
      end
    end
  rescue NameError
    self.class.class_variable_set(:@@fields, {})
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
    self.class.class_variable_get(:@@fields).each do |name, opts|
      val = instance_variable_get("@#{name}")
      if !opts.has_key?(:default) && !opts[:optional] && !val
        r << "#@location: required field `#{name}' undefined"
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
