# frozen_string_literal: true

require 'test_helper'
require 'active_support'

class FastUnderscoreTest < Minitest::Test
  def test_install
    path, = ''.method(:underscore).source_location
    assert_nil path
  end

  def test_basic
    assert_equal 'foo/bar_baz', 'foo::bar-baz'.underscore
    assert_equal 'foo_bar_baz', 'FooBarBaz'.underscore
    assert_equal 'foo123_bar', 'Foo123Bar'.underscore
  end

  def test_fuzzing
    source = %w[_ - : :: / 漢字 😊🎉] +
             ('a'..'z').to_a + ('A'..'Z').to_a + ('0'..'9').to_a

    500.times do
      word = Array.new(100) { source.sample }.join

      expected = ActiveSupport::Inflector.as_underscore(word)
      actual = word.underscore

      assert_equal expected, actual, "Word: #{word}"
    end
  end
end
