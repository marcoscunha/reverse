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

require 'ocm/interface'
require 'apes/cobject'
require 'apes/asmobject'
require 'apes/cxxobject'

require 'rubygems'
require 'term/ansicolor'
include Term::ANSIColor

class APECompilationUnit
  @@longer_name = ""
  attr :objects
  attr :update

  class CompilationError < RuntimeError
  end

  def initialize(interface, locals, globals)
    @interface = interface
    @objects = []
    @update = false

    @locals = locals
    @locals << (@interface.path + '/Headers')
    @locals << (@interface.path + '/Headers/Public')

    @globals = globals
    @globals << (@interface.path + '/Headers/Public')
    @globals << (@interface.path + '/Headers')

    if @interface.id.name.length >= @@longer_name.length
      @@longer_name = @interface.id.name
    end
  end

  def APECompilationUnit.createWith(interface, locals, globals)
    envs = []

    envs += APEASMObjectFile.envs() unless
      Dir.glob(interface.path + '/Sources/**/*.{s,S}').length == 0 

    envs += APECObjectFile.envs() unless
      Dir.glob(interface.path + '/Sources/**/*.c').length == 0 

    envs += APECXXObjectFile.envs() unless
      Dir.glob(interface.path + '/Sources/**/*.cpp').length == 0 

    envs.each do |env|

      if ENV[env] == nil
        raise CompilationError.new "Undefined #{env} variable."
      end

    end

    # If everything is OK, return an instance of the CcUnit
    APECompilationUnit.new(interface, locals, globals)
  end

  def updateObjectCache(cache)
    Dir.glob(@interface.path + '/Sources/**/*.{s,S}').each do |file|
      object = APEASMObjectFile.createWith(file, @interface, cache, @globals)
      @update = @update || object.update(@locals)
      @objects << object
    end
    Dir.glob(@interface.path + '/Sources/**/*.c').each do |file|
      object = APECObjectFile.createWith(file, @interface, cache, @globals)
      @update = @update || object.update(@locals)
      @objects << object
    end
    Dir.glob(@interface.path + '/Sources/**/*.cpp').each do |file|
      object = APECXXObjectFile.createWith(file, @interface, cache, @globals)
      @update = @update || object.update(@locals)
      @objects << object
    end
  end

  def build(verbose)
    unless verbose
      print @interface.id.name.blue
      (@@longer_name.length - @interface.id.name.length + 1).times { print ' ' }

      print '|'.bold
      @objects.each { |object| print object.update(@locals) ? ' '.on_cyan : ' '.on_green }
      print "|\e[#{@objects.length + 1}D".bold
    end

    # Compile the objects
    begin
      @objects.each do |o|
        if o.update(@locals) then
          begin
            o.build(verbose)
          rescue APEObjectFile::ObjectError => e
            path_array = @interface.path.split('/')
            path_array.delete_at(-1)
            base_path = path_array.join('/') + '/'
            message = e.message.split("\n")
            message.each { |l| l.slice!(base_path) }
            raise CompilationError.new(message.join("\n"))
          end
        else
          print "\e[C".on_green unless verbose
        end
      end
    rescue => e
      print "\r\e[2K" unless verbose
      raise e
    end

    print "\r\e[2K" unless verbose
  end

  def clean(verbose)
    @objects.each do |o|
      o.delete(verbose)
    end
  end
end
