require 'swd-mchck-bitbang'
require 'swd-buspirate'
begin
  require 'swd-ftdi'
rescue LoadError
  # Not required, we'll just lack support for FTDI
end

module BackendDriver
  class << self
    def create(name, opts)
      case name
      when 'ftdi', 'busblaster'
        FtdiSwd.new(opts)
      when 'buspirate'
        BusPirateSwd.new(opts)
      when 'mchck'
        MchckBitbangSwd.new(opts)
      end
    end

    def from_string_set(a)
      opts = {}
      a.each do |s|
        s.strip!
        name, val = s.split(/[=]/, 2) # emacs falls over with a /=/ regexp :/
        if !val || val.empty?
          raise RuntimError, "invalid option `#{s}'"
        end
        begin
          val = Integer(val)
        rescue
          # just trying...
        end
        opts[name.to_sym] = val
      end
      name = opts.delete(:name)
      create(name, opts)
    end

    def from_string(s)
      from_string_set(s.split(/:/))
    end
  end
end
