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

require 'rubygems'
require 'term/ansicolor'
include Term::ANSIColor

class APEGraphApplication < APEApplication

  def initialize
    super
    @optparse.banner = "Usage: apes-graph [options]"
  end

  def run(arguments = "")
    super(arguments)
    self.displayHelpAndExit unless @arguments.empty? or @arguments.count == 2

    begin
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
      # Compute the graph
      #

      deps = i.resolveDependences(interface_list)

      resolved, next_deps = [], []
      puts "digraph " + i.id.name + " {"

      deps.each do |d|
        if resolved.find { |r| r == d } == nil then
          next_deps = d.computeDependences(deps)
          puts "\t" + d.id.name + " [fontsize=16,style=filled];"
          next_deps.each do |n|
            puts "\t" + d.id.name + " -> " + n.id.name + ";"
          end
          resolved << d
        end
      end

      puts "}"
      return 0

    rescue Exception => e
      puts "\r\e[2K[#{e.class}]".red
      puts e.message
      puts e.backtrace if @verbose
    end

    return -1
  end

end
