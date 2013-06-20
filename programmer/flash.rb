$: << File.realpath('..', __FILE__)

require 'kinetis'
require 'backend-driver'
require 'optparse'

force_reset = false

OptionParser.new do |opts|
  opts.on("--reset", "Force system into reset") do |r|
    force_reset = true
  end
end.parse!

$stderr.puts "Attaching debugger..."
adiv5 = Adiv5.new(BackendDriver.from_string(ARGV[0]))
k = Kinetis.new(adiv5, force_reset)
$stderr.puts "done."

firmware = File.read(ARGV[1], :mode => 'rb')
address = Integer(ARGV[2])

$stderr.puts "Programming %d bytes of firmware to address %#x..." % [firmware.bytesize, address]
k.halt_core!
k.program(address, firmware) do |address, i, total|
  $stderr.puts "programming %#x, %d of %d" % [address, i, total]
end
$stderr.puts "done."
$stderr.puts "resetting..."
k.reset_system!
k.disable_debug!
k.continue!
$stderr.puts "done."
