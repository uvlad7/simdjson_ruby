# simdjson gem
[![Ruby](https://github.com/saka1/simdjson_ruby/workflows/Ruby/badge.svg)](https://github.com/saka1/simdjson_ruby/actions?query=workflow%3ARuby)
[![Gem Version](https://badge.fury.io/rb/simdjson.svg)](https://badge.fury.io/rb/simdjson)

A Ruby bindings for [simdjson](https://github.com/lemire/simdjson).

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'simdjson'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install simdjson

## Usage

`Simdjson.parse` returns a Hash (if successful).

```ruby
require 'simdjson'

p Simdjson.parse %|{"a": 12345}| # => {"a"=>12345}
Simdjson.parse %|abcd| # => raise Simdjson::ParseError
```

## Contributing

- Bug reports and pull requests are welcome on GitHub at https://github.com/saka1/simdjson_ruby.

### Formatter

Please use `clang-format` like this:

```
$ clang-format -style=file -i ext/simdjson/*
```

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).

## Development

    $ git clone git@github.com:uvlad7/simdjson_ruby.git --recurse-submodules

or

    $ git clone git@github.com:uvlad7/simdjson_ruby.git && git submodule update --init --recursive --depth 1
