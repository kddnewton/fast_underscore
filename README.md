# `String#underscore` Ruby Extension

[![Build Status](https://github.com/kddnewton/fast_underscore/workflows/Main/badge.svg)](https://github.com/kddnewton/fast_underscore/actions)
[![Gem Version](https://img.shields.io/gem/v/fast_underscore.svg)](https://rubygems.org/gems/fast_underscore)

`fast_underscore` is a C extension that provides a fast implementation of [ActiveSupport's `String#underscore` method](http://api.rubyonrails.org/classes/String.html#method-i-underscore).

## Is it fast?

At last check, these were the benchmarks (obtained by running `bin/bench` with Rails 6.0.2):

```
Warming up --------------------------------------
       ActiveSupport     5.000  i/100ms
      FastUnderscore    70.000  i/100ms
Calculating -------------------------------------
       ActiveSupport     50.564  (± 9.9%) i/s -    250.000  in   5.012563s
      FastUnderscore    707.691  (± 1.3%) i/s -      3.570k in   5.045375s

Comparison:
      FastUnderscore:      707.7 i/s
       ActiveSupport:       50.6 i/s - 14.00x  slower
```

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'fast_underscore'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install fast_underscore

### Usage with Rails

`ActiveSupport::Inflector#underscore`, in addition to underscoring the input will additionally take into account known acronyms. Since this can't be done at compile time, `FastUnderscore` will detect when `ActiveSupport` is loaded and take advantage of its knowledge of acronyms while still using the native extension.

Since the `#underscore` method is used so much throughout the Rails boot process (for autoloading dependencies, determining table names, determining inverse associations, etc.), it's best to hook into Rails as early as possible. As such, for the best results in your `Gemfile` add `require: false` to the `gem 'fast_underscore'` declaration and add `require 'fast_underscore'` to the bottom of `config/boot.rb`.

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake test` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/kddnewton/fast_underscore.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
