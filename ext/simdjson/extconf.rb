# frozen_string_literal: true

require 'mkmf'

$CXXFLAGS += ' -std=c++11 -Wno-register -march=native'

create_makefile('simdjson/simdjson')
