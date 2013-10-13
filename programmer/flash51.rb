$: << File.realpath('..', __FILE__)

require 'nrf51'
require 'backend-driver'

$stderr.puts "Attaching debugger..."
adiv5 = Adiv5.new(BackendDriver.from_string(ARGV[0]))

n = NRF51.new(adiv5)
$stderr.puts "done."

if ARGV[1] == '--mass-erase'
  $stderr.puts "Mass erasing chip..."
  n.mass_erase
  $stderr.puts "done."
else

  firmware = File.read(ARGV[1], :mode => 'rb')
  address = Integer(ARGV[2])

  $stderr.puts "Programming %d bytes of firmware to address %#x..." % [firmware.bytesize, address]
  n.halt_core!
  n.program(address, firmware) do |address, i, total|
    $stderr.puts "programming %#x, %d of %d" % [address, i, total]
  end
  $stderr.puts "done."
end

$stderr.puts "resetting..."
n.reset_system!
n.disable_debug!
n.continue!
$stderr.puts "done."
