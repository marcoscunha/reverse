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

class OCMArgument < OCMElement
  attr :type
  attr :direction

  def initialize(arguments)
    super(arguments)
    @type = arguments.pop
    @direction = arguments.pop
  end

  def self.createFromXML(node, arguments = [])
    arguments.push(node["direction"])
    arguments.push(node["type"])
    return super(node, arguments)
  end

  def to_s
    return @name.underscore + ': ' + @direction.to_s.red + ' ' + @type.blue
  end

  def eql?(arg)
    return false if arg == nil
    return super(arg) && @type == arg.type && @direction == arg.direction
  end

  def hash
    return [@name, @type, @direction].hash
  end

  private

  alias :== :eql?
end

