$: << File.realpath('..', __FILE__)
require 'kinetis'
require 'nrf51'
require 'log'

module Device
  class << self

    def detect(adiv5)

      Log(:device, 1){ "detecting device" }

      ret = NRF51.detect(adiv5)
      if !ret
        ret = Kinetis.detect(adiv5)
      end

      if !ret
        raise RuntimeError, "Unable to detect device."
      end

      return ret

    end

  end

end