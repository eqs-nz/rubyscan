#!/usr/bin/ruby
require 'mkmf'

def have_deps
  @deps.each do |x|
    if x.is_a? RuntimeError
      puts "error: #{x}"
      return false
    end
  end
  true
end

def error(msg)
  new RuntimeError(msg)
end

@deps = [
    (have_library('hs', 'hs_compile') or error('missing libhs')),
    (have_library('hs_runtime', 'hs_scan') or error('missing libhs_runtime')),
    (have_header('hs/hs.h') or error('missing hs/hs.h'))
]


y = ->(f) {
  ->(g) {
    g.(g)
  }.(->(g) {
    f.(->(*args) {
      g.(g).(*args)
    })
  })
}

traverse = y.(lambda { |recurse|
  lambda { |path|
    lambda { |file|
      return if file =~ /^\.+/
      if Dir.exist? "#{path}/#{file}"
        # ruby extensions are flat in structure for now
        # return Dir.new("#{path}/#{file}").entries.collect &recurse.("#{path}/#{file}")
      elsif File.exist? "#{path}/#{file}"
        puts "saw #{path}/#{file}"
        $srcs << "#{path}/#{file}" if file =~ /.*\.c$/
      end
    }
  }
})

$srcs = []

Dir.new($srcdir).entries.each &traverse.($srcdir)

if have_deps
  create_header
  create_makefile 'rubyscan/rubyscan'
end
