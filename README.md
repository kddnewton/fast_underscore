# `String#underscore` Ruby Extension

[![Build Status](https://travis-ci.org/kddeisz/fast_underscore.svg?branch=master)](https://travis-ci.org/kddeisz/fast_underscore)
[![Gem Version](https://img.shields.io/gem/v/fast_underscore.svg)](fast_underscore)

`fast_underscore` is a simple C extension which provides a fast implementation of [Active Support's `String#underscore` method](http://api.rubyonrails.org/classes/String.html#method-i-underscore).

## Do I need this?

Maybe! Run a stack profiler like [`ruby-prof`](https://github.com/ruby-prof/ruby-prof). If `String#underscore` or `ActiveSupport::Inflector#underscore` is coming up near the top of the list, this gem might be for you! If not, you probably don't need it. Either way, your milage may vary depending on the type of strings on which you're calling `underscore`.

## Usage with Rails

`ActiveSupport::Inflector#underscore`, in addition to underscoring the input will additionally take into account known acronyms. Since this can't be done at compile time, `FastUnderscore` will detect when `ActiveSupport` is loaded and take advantage of its knowledge of acronyms while still using the native extension.

Since the `#underscore` method is used so much throughout the Rails boot process (for autoloading dependencies, determining table names, determining inverse associations, etc.), it's best to hook into Rails as early as possible. As such, for the best results in your `Gemfile` add `require: false` to the `gem 'fast_underscore'` declaration and add `require 'fast_underscore` to the bottom of `config/boot.rb`.

## Is it fast?

At last check, these were the benchmarks (obtained by running `bin/benchmark`):

### Rails 5.1.4

```
Warming up --------------------------------------
       ActiveSupport     2.000  i/100ms
      FastUnderscore    64.000  i/100ms
Calculating -------------------------------------
       ActiveSupport     28.839  (± 6.9%) i/s -    144.000  in   5.019140s
      FastUnderscore    650.151  (± 7.7%) i/s -      3.264k in   5.056094s

Comparison:
      FastUnderscore:      650.2 i/s
       ActiveSupport:       28.8 i/s - 22.54x  slower
```

### Rails 5.2.0

```
Warming up --------------------------------------
       ActiveSupport     5.000  i/100ms
      FastUnderscore    66.000  i/100ms
Calculating -------------------------------------
       ActiveSupport     50.055  (± 6.0%) i/s -    250.000  in   5.016223s
      FastUnderscore    655.181  (± 2.1%) i/s -      3.300k in   5.038968s

Comparison:
      FastUnderscore:      655.2 i/s
       ActiveSupport:       50.1 i/s - 13.09x  slower
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

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake test` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/kddeisz/fast_underscore.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
