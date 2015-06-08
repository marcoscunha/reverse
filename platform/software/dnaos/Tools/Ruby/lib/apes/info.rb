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
require 'ocm/id'
require 'ocm/interface'

require 'rubygems'
require 'term/ansicolor'
include Term::ANSIColor

class APEInfoApplication < APEApplication

  def initialize
    super
    @optparse.banner = "Usage: apes-info [options] {<name> <version>}"
  end

  def run(arguments = "")
    super(arguments)
    self.displayHelpAndExit unless @arguments.empty? or @arguments.count == 2

    begin
      case @arguments.count
      when 0 
        i = OCMInterface.createFromXMLFileAtPath(Dir.pwd, @verbose)
        raise Exception.new('No interface in this directory.') if i == nil

      when 2
        id = OCMId.new(@arguments[0], @arguments[1])
        i = APELibraryParser.findInterfaceWith(id)
        raise Exception.new(id.to_s + ': No such interface.') if i == nil

      end

      i.display
      return 0

    rescue Exception => e
      puts "\r\e[2K[#{e.class}]".red
      puts e.message
      puts e.backtrace if @verbose
    end

    return -1
  end
end
