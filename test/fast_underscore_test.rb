require "test_helper"

class FastUnderscoreTest < Minitest::Test
  def test_underscore
    assert_equal 'foo/bar_baz', 'foo::bar-baz'.underscore
  end
end
