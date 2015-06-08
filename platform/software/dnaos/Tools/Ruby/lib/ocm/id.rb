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
require 'nokogiri'
require 'term/ansicolor'
include Term::ANSIColor

class OCMId < OCMElement
  attr :name
  attr :version

  def initialize(name, version)
    @name = name
    @version = version
  end

  def OCMId.createFromXML(node)
    name = node["name"]
    version = node["version"]
    return self.new(name, version)
  end

  def to_s
    return @name.blue + ', ' + @version.green
  end

  def eql?(id)
    return false if id == nil
    return @name == id.name && @version == id.version
  end

  def hash
    return [@name, @version].hash
  end

  alias :== :eql?
end

