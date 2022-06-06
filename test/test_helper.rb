# frozen_string_literal: true

at_exit { GC.start }

$LOAD_PATH.unshift File.expand_path("../lib", __dir__)
require "fast_underscore"

require "active_support"
require "minitest/autorun"
