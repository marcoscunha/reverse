# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'apes/application'
require 'apes/parser'
require 'apes/compiler'
require 'apes/linker'
require 'apes/object'
require 'ocm/id'
require 'ocm/interface'

require 'rubygems'
require 'term/ansicolor'
include Term::ANSIColor

class APEComposeApplication < APEApplication

  def initialize
    super
    @optparse.banner = "Usage: apes-compose [options] {<name> <version>}"
    @optparse.on("-c", "--clean",
                 "Clean the interface graph objects.") do |c|
      @clean = true
    end
  end

  def run(arguments = "")
    super(arguments)
    self.displayHelpAndExit unless @arguments.empty? or @arguments.count == 2

    begin

      #
      # Create the object cache directory
      #

      Dir.mkdir(CACHE_DIRECTORY) unless File.exist?(CACHE_DIRECTORY)

      #
      # Find the interface to build
      #

      case @arguments.count
      when 0 
        i = OCMInterface.createFromXMLFileAtPath(Dir.pwd, @verbose)
        raise Exception.new('No interface in this directory.') if i == nil

        interface_list = APELibraryParser.getInterfaceList(@verbose)
        interface_list << i if APELibraryParser.findInterfaceWith(i.id) == nil

      when 2
        id = OCMId.new(@arguments[0], @arguments[1])
        i = APELibraryParser.findInterfaceWith(id)
        interface_list = APELibraryParser.getInterfaceList(@verbose)
      end

      #
      # Check the interface dependences
      #

      deps = i.resolveDependences(interface_list)
      raise Exception.new("Cannot resolve " + i.id.to_s) if deps.empty?

      #
      # Build the interface
      #

      compilers = []

      deps.each do |d|
        if not d.wrapper then
          locals, globals = [], []

          d.resolveDependences(deps).each do |l|
            globals << "#{l.path}/Headers/Public" unless l.wrapper
          end

          d.computeDependences(deps).each do |l|
            locals << "#{l.path}/Headers/Public" unless l.wrapper
          end

          unit = APECompilationUnit.createWith(d, locals, globals)
          compilers << unit

          unit.updateObjectCache(CACHE_DIRECTORY)

          if @clean then
            unit.clean(@verbose)
          else
            unit.build(@verbose) if unit.update
          end
        end
      end

      #
      # If we are compiling, link all the objects altogether
      #

      unless @clean then
        binary = "#{Dir.pwd}/#{i.id.name}"
        APELinkUnit.link(binary, CACHE_DIRECTORY, compilers, @verbose)
      end

      return 0

    rescue Exception => e
      puts "\r\e[2K[#{e.class}]".red
      puts e.message
      puts e.backtrace if @verbose
    end

    return -1
  end

  private

  CACHE_DIRECTORY = ENV['HOME'] + "/.apes"

end
