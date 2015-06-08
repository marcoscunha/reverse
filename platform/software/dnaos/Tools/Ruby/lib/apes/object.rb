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

require 'rubygems'
require 'yaml'
require 'digest'
require 'popen4'
require 'pp'

class APEObjectFile
  attr :update
  attr :object
  attr :flags
  attr :interface
  attr :version
  attr :source

  class ObjectError < RuntimeError
  end

  def initialize(source, interface, version, sandbox, includes, flags)
    @source = source
    @interface = interface
    @version = version
    @includes = includes
    @sandbox = sandbox
    @object = sandbox + '/object'
    @description = sandbox + '/description'
    @flags = flags
    @update = false
  end

  def APEObjectFile.createWith(source, interface, cache, includes, flags, downClass)

    #
    # Compute the object SHA1
    #

    signature = interface.id.name + ':' + source + ':'
    signature += interface.id.version + ':' + flags
    sha1 = Digest::SHA1.hexdigest signature
    sandbox = cache + '/' + sha1

    #
    # Generate the description file
    #
    
    if not File.exist?(sandbox) then 
      Dir.mkdir(sandbox)

      #
      # Compute the description hash
      #
      
      values = [ flags, includes.join(' '), interface.id.name, 
        source, interface.id.version ]
      configuration = Hash[*CONFIGURATION_KEYS.zip(values).flatten]

      #
      # Generate the YAML description file
      #

      File.open(sandbox + '/description', 'w+') do |f|
        YAML.dump(configuration, f)
      end
    elsif not File.directory?(sandbox)
      raise ObjectError.new "Invalid cache object: prune invalid objects."
    end

    #
    # Create the object file
    #

    return downClass.new(source, interface.id.name, interface.id.version,
                      sandbox, includes, flags)
  end

  def APEObjectFile.createFrom(sandbox)
    object = nil

    if File.exist?(sandbox) && File.directory?(sandbox) then
      # Load the object's configuration file, check the keys
      # and create the object file
      #

      if File.exist?(sandbox + '/description') then
        config = File.open(sandbox + '/description', 'r') do |f|
          YAML.load(f)
        end

        if config.keys.sort == CONFIGURATION_KEYS then
          object = APEObjectFile.new(config['source'], config['interface'],
                                     config['version'], sandbox,
                                     config['includes'], config['flags'])
        end
      end
    end

    return object
  end

  def SHA1
    signature = @interface + ':' + @source + ':'
    signature += @version + ':' + @flags
    return Digest::SHA1.hexdigest signature
  end

  def validate
    id = OCMId.new(@interface, @version)
    interface = APELibraryParser.findInterfaceWith(id)
    return interface != nil
  end

  def update(local_deps)
    update = true
    has_modification = false

    # Check if the object has to be built
    if File.exist?(@object) then 
      # Get the Modification Time of the object
      file = File.new(@object)
      object_time = file.mtime
      file.close

      # Get the Modification Time of the source file
      file = File.new(@source)
      source_time = file.mtime
      file.close

      # Check if an update is needed
      if object_time > source_time then
        local_deps.each do |inc|

          # Check each header file of the dependence
          updated_files = Dir.glob(inc + '/**/*.h').find do |f| 
            file = File.new(f)
            dep_time = file.mtime
            file.close
            object_time < dep_time
          end 

          # If at least one of them is modified, we recompile
          if updated_files != nil then
            has_modification = true
            break
          end
        end 

        update = false if not has_modification
      end
    end

    @update = update
    return update
  end

  def build(verbose)
    status = 0

    # Build the command array
    cmd_array = [ENV['APES_COMPILER']]
    cmd_array << "-c -o #{@object}"
    cmd_array << ENV['APES_CC_FLAGS']
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
        message = "Cannot execute " + ENV['APES_COMPILER']
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

  def delete(verbose)
    if File.exist?(@object) then
      File.delete(@object)
    end

    if File.exist?(@description) then
      File.delete(@description)
    end

    Dir.delete(@sandbox)
  end

  private

  CONFIGURATION_KEYS = [ "flags", "includes", "interface", "source", "version" ]
end
