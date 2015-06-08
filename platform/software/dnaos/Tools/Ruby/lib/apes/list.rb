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

class APEListApplication < APEApplication

  def intialize
    super
    @optparse.banner = "Usage: apes-list [options]"
  end

  def run(arguments = "")
    super(arguments)
    self.displayHelpAndExit unless @arguments.empty?

    begin
      APELibraryParser.getInterfaceList(@verbose) { |c| puts c.id }
      return 0

    rescue Exception => e
      puts "\r\e[2K[#{e.class}]".red
      puts e.message
      puts e.backtrace if @verbose
    end

    return -1
  end

end
