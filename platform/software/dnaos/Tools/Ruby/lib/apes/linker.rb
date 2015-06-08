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

require 'apes/compiler'
require 'rubygems'
require 'popen4'
require 'term/ansicolor'
include Term::ANSIColor

class APELinkUnit

  class LinkError < RuntimeError
  end

  def APELinkUnit.link(name, buildir, compilers, verbose)
    # Check if the necessary env variables are present
    #

    if ENV['APES_LINKER'] == nil
      raise LinkError.new "Undefined APES_LINKER variable."
    end

    if ENV['APES_LINKER_FLAGS'] == nil
      raise LinkError.new "Undefined APES_LINKER_FLAGS variable."
    end

    #
    # We get the complete object list
    #

    objects = []
    compilers.each { |cc| objects += cc.objects }

    #
    # We try to link the objects
    #

    cmd = [ENV['APES_LINKER']]
    cmd << "-o #{name}"
    cmd << ENV['APES_LINKER_FLAGS']
    objects.each { |o| cmd << "#{o.object}" }
    command = cmd.join(' ')

    #
    # Deal with calling the linker
    #

    puts command if verbose

    before = Time.now

    stdout, stderr = [], []
    status = POpen4::popen4(command) do |out,err|
      eof_out = eof_err = false
      stdout = stderr = String.new

      while (!eof_out && !eof_err) do
        begin
          stdout += out.read_nonblock(128) if !eof_out
        rescue EOFError
          eof_out = true
        rescue Errno::EAGAIN
          # retrying
        end

        begin
          stderr += err.read_nonblock(128) if !eof_err
        rescue EOFError
          eof_err = true
        rescue Errno::EAGAIN 
          # retrying
        end
      end
    end

    success = File.exist?(name) ? (File.mtime(name).to_i >= before.to_i) : false 

    #
    # Deal with the errors
    #

    print "\r\e[2K" unless verbose

    if status == nil
      message = "Cannot execute " + ENV['APES_LINKER']
      message += ", no such file or directory"
      raise LinkError.new message
    elsif status != 0 || !success
      raise LinkError.new(stderr)
    end
  end
end

