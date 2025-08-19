# frozen_string_literal: true

require 'mkmf'

$CXXFLAGS += ' -std=c++17 -Wno-register -march=native'

create_makefile('simdjson/simdjson')
