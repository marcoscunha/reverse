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

require 'apes/parser'
require 'ocm/interface'
require 'ocm/id'
require 'apes/object'

require 'rubygems'
require 'yaml'
require 'digest'
require 'popen4'
require 'pp'

class APEASMObjectFile < APEObjectFile
  attr :object
  attr :flags
  attr :interface
  attr :version
  attr :source

  def initialize(source, interface, version, sandbox, includes, flags)
    super(source, interface, version, sandbox, includes, flags)
  end

  def APEASMObjectFile.envs()
    return [ 'APES_ASSEMBLER', 'APES_AS_FLAGS', 'APES_AS_OPTIMIZATIONS' ]
  end

  def APEASMObjectFile.createWith(source, interface, cache, includes)
    interface_var = interface.id.name.upcase + '_AS_FLAGS'
    flags = ENV['APES_AS_FLAGS'] + ' ' + 
      ENV['APES_AS_OPTIMIZATIONS'] + ' ' +
      (ENV[interface_var] != nil ? ENV[interface_var] : "");
    return super(source, interface, cache, includes, flags, APEASMObjectFile)
  end

  def build(verbose)
    status = 0

    # Build the command array
    cmd_array = [ENV['APES_ASSEMBLER']]
    cmd_array << "-c -o #{@object}"
    cmd_array << ENV['APES_AS_FLAGS']
    cmd_array << @flags

    cmd_array << @includes.collect { |d| '-I' + d }.join(' ')
    cmd_array << @source
    command = cmd_array.join(' ')

    puts command if verbose

    # Execute the command
    if @update then
      stdout, stderr = [], []
      GC.disable
      status = POpen4::popen4(command) do |out,err|
        stdout = out.readlines
        stderr = err.readlines
      end
      GC.enable

      if status == nil
        message = "Cannot execute " + ENV['APES_ASSEMBLER']
        message += ", no such file or directory"
        raise ObjectError.new message
      elsif status != 0
        raise ObjectError.new(stderr.join)
      end

      print verbose ? stdout.join : ' '.on_green
    else
      print "\e[C".on_green unless verbose
    end
  end

end
