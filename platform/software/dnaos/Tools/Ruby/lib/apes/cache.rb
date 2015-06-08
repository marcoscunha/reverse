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
require 'apes/object'

require 'rubygems'
require 'fileutils'
require 'term/ansicolor'
include Term::ANSIColor

class APECacheApplication < APEApplication

  def initialize
    super
    @optparse.banner = "Usage: apes-cache [options]"

    @optparse.on("-s", "--show",
                 "Shows the cache's content.") do |s|
      @cmd = :show
    end

    @optparse.on("-p", "--purge",
                 "Purges the cache.") do |p|
      @cmd = :purge
    end

    @optparse.on("-x", "--prune",
                 "Prunes invalid cache entries.") do |x|
      @cmd = :prune
    end
  end


  def run(arguments = "")
    super(arguments)
    self.displayHelpAndExit unless @arguments.empty?
    self.displayHelpAndExit if @cmd == nil

    begin
      case @cmd
      when :show
        Dir.glob(CACHE_DIRECTORY + "/*").each do |file|
          object = APEObjectFile.createFrom(file)
          if object == nil then
            puts file.split('/').last.blue.bold + " [INVALID]".red.bold
          else
            print object.SHA1.blue.bold + ' '
            puts object.validate ? "[OK]".green.bold : "[UNREFERENCED]".red.bold
            puts '| Interface: '.bold + object.interface + ', ' + object.version
            puts '| Source file: '.bold + object.source.split('/').last
            puts '| Flags: '.bold + object.flags
          end
        end

      when :prune, :purge
        Dir.glob(CACHE_DIRECTORY + "/*").each do |file|
          object = APEObjectFile.createFrom(file)
          if object == nil then
            FileUtils.rm_rf(file)
            puts file.split('/').last.blue + " [DELETED]".red if @verbose
          elsif @cmd == :purge || (@cmd == :prune && ! object.validate) then
            object.delete(@verbose)
            if @verbose then
              puts object.SHA1.blue + " [DELETED]".red
              puts '| Interface: ' + object.interface + ', ' + object.version
              puts '| Source file: ' + object.source.split('/').last
              puts '| Flags: ' + object.flags
            end
          end
        end
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

  COMMANDS = %w(show purge prune)
  CACHE_DIRECTORY = ENV['HOME'] + "/.apes"

end
