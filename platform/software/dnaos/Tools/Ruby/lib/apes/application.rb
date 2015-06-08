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

require 'rubygems'
require 'optparse'

class APEApplication

  def initialize
    @arguments = []
    @verbose = false
    @optparse = OptionParser.new

    @optparse.on("-v", "--verbose",
                 "Enables verbose mode.") do |v|
      @verbose = true
    end

    @optparse.on("-h", "--help",
                 "Displays this help.") do |h|
      @help = true
    end
  end

  def run(args)
    begin
      @arguments = @optparse.parse!(args)
    rescue Exception => e
      puts "\r\e[2K[#{e.class}]".red
      puts e.message
      puts e.backtrace
      exit
    end

    self.displayHelpAndExit if @help
    return 0


  end

  def displayHelpAndExit
    puts @optparse
    exit 0
  end

end
