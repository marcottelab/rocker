require 'rubygems'
require 'mkmf-rice'

dir_config("rockerxx")
dir_config("boost")

have_library("stdc++")
have_library("pqxx")
# have_library("boost")
have_library("boost_filesystem")
if RUBY_VERSION =~ /1.9/ then
  $CPPFLAGS += " -DRUBY_19"
end

$CPPFLAGS += " -DRICE"
$CXXFLAGS += " -O3"

create_makefile('rockerxx')
