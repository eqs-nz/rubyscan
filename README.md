# Rubyscan

Rubyscan provides a feature-rich, native Ruby language interface to the [Hyperscan](https://github.com/01org/hyperscan) low-level pattern matching library. It is written entirely in C, and currently only supports the MRI Ruby interpreter. Future releases may add support for other interpreter platforms (such as JRuby).
  
Hyperscan is a highly-concurrent, event oriented, multiple regular expression matching library for the C language. While it is primarily targeted at deep packet inspection stacks, its functionality offers potential value to many other monitoring-oriented applications. Hyperscan is developed by Intel and licensed under the BSD license ([details](https://github.com/01org/hyperscan/blob/master/LICENSE)).
 
The Rubyscan library provides classes for the definition and compilation of regular expressions, and a scanner interface capable of simple and complex scanning scenarios. Match events are handled asynchronously without incurring Ruby interpreter overhead, and passed to a handler block in a fashion familiar to Ruby programmers.

# License

Rubyscan is licensed under the Simplified BSD License. See the LICENSE file in the project repository.

## Usage

Compiling a simple expression:

```ruby
require 'rubyscan'
include Scan::Compile

expression = Expression.new("hello")
expression.set_flags :caseless
db = Compiler.new.compile expression

...
```

Scanning for a simple expression:
```ruby
...

include Scan::Runtime

scanner = Scanner.new

scanner.phases << Scanner::Unit.new db { |source, event|
  puts "#{event.line}"
}

scanner.running = true
scanner.scan 'Hi, planet?'
scanner.scan 'Hello, world!'
scanner.running = false
```

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/eqs-nz/rubyscan.

