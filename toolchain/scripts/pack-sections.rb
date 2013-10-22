#!/usr/bin/env ruby

class Objdump < Array
  class Section
    attr_accessor :file, :name, :size, :flags, :align, :addr

    def to_s
      s = "#{@file || '*'} (#@name)"
      if addr
        s = ". = %#x; KEEP(%s)" % [addr, s]
      end
      s
    end
  end

  def initialize(fs)
    super(0)
    fs.each do |f|
      parse(f)
    end
  end

  def parse(f)
    rec = nil
    objdump = ENV["OBJDUMP"] || "arm-none-eabi-objdump"
    `#{objdump} -h #{f}`.split("\n").each do |l|
      case l
      when /^ *\d+ ([.][^\s]+)\s+([0-9a-f]{8})\s+.*?2[*][*](\d+)$/
        rec = Section.new
        rec.file = f
        rec.name = $1
        rec.size = $2.to_i(16)
        rec.align = $3.to_i
      when /^\s+((?:[A-Z]+, )*[A-Z]+)/
        rec.flags = $1.split(", ")
        push(rec)
        rec = nil
      end
    end
  end
end

class Knapsack
  def self.calc(el, w)
    keep = {}
    v = [0] * (w + 1)
    el.each do |e|
      nv = []
      (w + 1).times do |w|
        nsize = e.size + v[w - e.size]
        if e.size < w && nsize > v[w]
          nv << nsize
          keep[[e, w]] = true
        else
          nv << v[w]
          keep[[e, w]] = false
        end
      end
      v = nv
    end

    k = w
    r = []
    el.reverse.each do |e|
      if keep[[e, k]]
        r << e
        k -= e.size
      end
    end

    r
  end
end


class Linker
  def initialize(sect, fixed)
    @all = sect.select do |s|
      s.flags.include?("READONLY") &&
        s.flags.include?("LOAD") &&
        s.size > 0
    end

    @fixed = fixed.sort_by{|e| e.addr}
    allfixed, allrest = @all.partition do |s|
      @fixed.map{|fs| fs.name}.include?(s.name)
    end
    @all = allrest
    @fixed.each do |s|
      s.size = allfixed.select{|as| as.name == s.name}.inject(0){|t, as| t + as.size}
    end
  end

  def pack
    res = []

    remaining = @all
    @fixed.each_cons(2) do |startsect, endsect|
      space = endsect.addr - startsect.addr - startsect.size

      best = Knapsack.calc(remaining, space)
      best.sort_by!{|s| -s.align}
      remaining = remaining - best

      res << startsect
      res << "/* available space #{space}, packed in #{best.inject(0){|t,s| t+s.size}} */"
      res += best
    end
    res << @fixed[-1] if @fixed[-1]
    res += remaining
  end
end


if $0 == __FILE__
  require 'optparse'

  fixed_sects = []
  outfile = $stdout
  OptionParser.new do |opts|
    opts.banner = "usage: #{__FILE__} [-s addr:sects [-s addr:sects ...]] [objects...]"

    opts.on("-s", "--section ADDR:SECTS",
            /^(0x[0-9a-f]+|[0-9]+)[:=]([^\s]+)$/i,
            "Define fixed location for SECTION at ADDR") do |saddr|
      s = Objdump::Section.new
      s.name = saddr[2]
      s.addr = Integer(saddr[1])
      fixed_sects << s
    end

    opts.on("-o", "--output FILE",
            "Write sections to FILE") do |file|
      outfile = File.open(file, "w")
    end
  end.parse!

  all_sect = Objdump.new ARGV

  l = Linker.new(all_sect, fixed_sects)

  outfile.puts "/* This file has been automatically generated */"
  outfile.puts l.pack.join("\n")
end
