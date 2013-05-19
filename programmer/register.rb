require 'set'
require 'log'

module Peripheral
  class ValueError < RuntimeError
  end


  class CachingProxy
    attr_accessor :read_set, :write_set

    def initialize(field)
      @read_set = {}
      @write_set = {}
      if field.respond_to? :each
        field.each_with_index do |d, i|
          addr = i * 4
          if Array === d
            addr, d = d
          end
          @read_set[addr] = d
        end
      else
        @backing = field
      end
    end

    def [](idx)
      @write_set[idx] || @read_set[idx] ||= @backing[idx]
    end

    def []=(idx, val)
      @write_set[idx] = val
    end
  end

  class VectorProxy
    attr_reader :size

    def initialize(base, name, size)
      @base, @name, @size = base, name, size
    end

    def [](idx)
      @base.send("#{@name}[#{idx}]")
    end

    def []=(idx, val)
      @base.send("#{@name}[#{idx}]", val)
    end

    def each
      @size.times do |i|
        yield self[i]
      end
    end
  end

  class BackingProxy
    def initialize(backing, offset = 0)
      @backing, @offset = backing, offset
    end

    def [](idx)
      @backing.get_backing(@offset + idx)
    end

    def []=(idx, val)
      @backing.set_backing(@offset + idx, val)
    end
  end

  class FieldInfo
    attr_accessor :name, :bit_offset, :width, :desc
    attr_accessor :enum, :inverse_enum

    attr_accessor :offset, :shift, :mask, :complete

    def initialize(name, range, opts)
      @name = name

      if Array === range
        @offset, range = range
      end

      if Range === range
        @width = range.begin - range.end + 1
        @bit_offset = range.end
      else
        @width = 1
        @bit_offset = range
      end

      @offset ||= 0
      if @offset % 4 != 0
        @bit_offset += @offset % 4 * 8
        @offset -= @offset % 4
      end

      raise ValueError, "field #{name} exceeds word size" if @bit_offset + @width > 32

      if opts[:absolute]
        @shift = 0
        @mask = ((1 << width) - 1) << @bit_offset
      else
        @shift = @bit_offset
        @mask = (1 << width) - 1
      end
      @complete = true if @width == 32
    end
  end

  module FieldManager
    module ClassMethods
      def add_field_constant(name, val)
        name = name.to_s
        if name[0].upcase != name[0]
          name = name.capitalize
        end
        if name[-1] == "?"
          name.slice!(-1)
        end
        self.const_set(name, val) unless self.const_defined?(name)
      rescue NameError => e
        #puts "could not add field name #{name}: #{e}"
      end

      def add_field(name, range, opts)
        self.class_eval do
          f = FieldInfo.new(name, range, opts)

          add_field_constant(f.name, f.offset) if f.complete

          fields = {}
          begin
            fields = self.class_variable_get(:@@fields)
          rescue NameError
            self.class_variable_set(:@@fields, fields)
          end
          fields[f.name] = f

          begin
            # this will return class Range the first time...
            field_offsets = self.class_variable_get(:@@offsets)
          rescue NameError
            field_offsets = Set.new
            self.class_variable_set(:@@offsets, field_offsets)
          end
          field_offsets << f.offset

          yield f
        end
      end

      def add_unsigned(name, range, opts)
        self.add_field(name, range, opts) do |f|
          define_method name do
            read_field f
          end
          define_method "#{name}=" do |val|
            set_field f, val
          end
        end
      end

      def add_enum(name, range, enum, opts)
        self.add_field(name, range, opts) do |f|
          enum.each do |k, v|
            if v & ~f.mask != 0
              raise ValueError, "#{k} => #{v} too large for #{f.name} width of #{f.width}"
            end
          end

          f.enum = enum
          f.inverse_enum = Hash[enum.map{|k,v| [v,k]}]

          define_method name do
            f.inverse_enum[read_field(f)]
          end
          define_method "#{name}=" do |sval|
            set_field(f, f.enum[sval] || sval)
          end
        end
      end
    end

    def self.included(othermod)
      othermod.extend(ClassMethods)
    end

    def read_field(f)
      v = get_backing(f.offset)
      v = v >> f.shift
      v = v & f.mask
      Log :reg, 2, "reading %s.%s = %#010x" % [self.class, f.name, v]
      v
    end

    def set_field(f, val)
      if val & ~f.mask != 0
        raise ValueError, "#{val} overflowing #{f.name} width of #{f.width}"
      end
      v = val
      if !f.complete
        v = get_backing(f.offset)
        v = v & ~(f.mask << f.shift)
        v = v | (val << f.shift)
      end
      Log :reg, 2, "writing %s.%s = %#010x" % [self.class, f.name, val]
      set_backing(f.offset, v)
      val
    end

    def get_address(offset=0)
      addr = @address
      begin
        addr ||= self.class.class_variable_get(:@@default_address)
      rescue NameError
      end
      addr ||= 0
      addr + offset
    end

    def get_backing(offset)
      v = @backing[get_address(offset)]
      Log :reg, 2, "reading %s @ %#010x backing %s = %#010x" %
        [@backing, get_address(offset), self.class, v]
      v
    end

    def set_backing(offset, val)
      Log :reg, 2, "writing %s @ %#010x backing %s = %#010x" %
        [@backing, get_address(offset), self.class, val]
      @backing[get_address(offset)] = val
    end

    def to_s
      s = "<%s:%#x address: %#x backing: %s\n" % [self.class, self.object_id, get_address(0), @backing]
      fs = self.class.class_variable_get(:@@fields)
      s += fs.values.sort_by{|f| f.offset * 32 + f.bit_offset}.map do |f|
        v = nil
        begin
          v = self.send(f.name)
        rescue Exception => e
          v = "(exception occured: #{e})"
        end
        v = "%#x" % v if Integer === v
        va = "#{v}".split("\n")
        fs = "  #{f.name}: #{va[0]}"
        if va.count > 1
          fs = ([fs] + va[1..-1]).join("\n  ")
        end
        fs
      end.join("\n")
      s += ">"
    end
  end

  class BitField
    include FieldManager

    def self.unsigned(name, range, opts={})
      add_unsigned(name, range, opts)
    end

    def self.bool(name, pos, opts={})
      add_enum(name, pos, {true => 1, false => 0}, opts)
    end

    def self.enum(name, range, enum)
      add_enum(name, range, enum, {})
    end

    def initialize
      @backing = CachingProxy.new(nil)
      offsets = self.class.class_variable_get(:@@offsets)
      offsets.each do |ofs|
        @backing[ofs] = 0
      end
    end

    def self.shadow(backing, offs=0)
      c = self.allocate
      c.instance_eval do
        @backing = backing
        @address = offs
      end
      c
    end

    def initialize_copy(other)
      value = other.map{|o, d| [other.get_address(o), d]}
      @backing = CachingProxy.new(value)
    end

    def transact
      proxy = CachingProxy.new(@backing)
      f = self.class.shadow(proxy)
      yield f
      proxy.write_set.each do |o, d|
        set_backing(o, d)
      end
    end

    def replace!(val)
      val.each_with_index do |i, d|
        set_backing(i, d)
      end
    end

    def zero!
      self.class.class_variable_get(:@@offsets).map do |o|
        set_backing(o, 0)
      end
    end

    def to_i
      offset = self.class.class_variable_get(:@@offsets)
      raise RuntimeError, "not a single word register #{offset.to_a}" if offset.count > 1
      get_backing(0)
    end

    include Enumerable

    def each
      offsets = self.class.class_variable_get(:@@offsets)
      m = offsets.map do |o|
        yield [o, get_backing(o)]
      end
    end
  end

  include FieldManager

  module ClassMethods
    include FieldManager::ClassMethods

    def add_vector(name, offset, len)
      add_field_constant(name, offset)
      define_method name do
        VectorProxy.new(self, name, len)
      end
      define_method "#{name}=" do |other|
        other.each_with_index do |d, i|
          ooffs = i * 4
          if Array === d
            ooffs, d = d
          end
          set_backing(offset + ooffs, d)
        end
      end
      len.times do |i|
        ename = "#{name}[#{i}]"
        eoffs = offset + i * 4
        yield ename, eoffs
      end
    end

    def register(name, offset, opts={}, &block)
      cl = Class.new(BitField)
      add_field_constant(name, cl)
      cl.class_eval &block
      add_field(name, [offset, 31..0], {}) do |f|
        # nothing to do, I think
      end
      if opts[:vector]
        self.add_vector(name, offset, opts[:vector]) do |ename, eoffs|
          define_method ename do
            cl.shadow(BackingProxy.new(self), eoffs)
          end
        end
      else
        define_method name do
          cl.shadow(BackingProxy.new(self), offset)
        end
      end
    end

    def unsigned(name, offset, opts={})
      if opts[:vector]
        self.add_vector(name, offset, opts[:vector]) do |ename, eoffs|
          self.unsigned(ename, eoffs)
        end
      else
        add_unsigned(name, [offset, 31..0], {})
      end
    end

    def enum(name, offset, enum)
      add_enum(name, [offset, 31..0], enum)
    end

    def default_address(val)
      self.class_variable_set(:@@default_address, Integer(val))
    end
  end

  def self.included(othermod)
    othermod.extend(ClassMethods)
  end

  attr_reader :address
  def initialize(mem, address=nil)
    @backing = mem
    @address = address if address
  end

  def read(addr, opt={})
    @backing.read(get_address(addr), opt)
  end

  def write(addr, val)
    @backing.write(get_address(addr), val)
  end
end
