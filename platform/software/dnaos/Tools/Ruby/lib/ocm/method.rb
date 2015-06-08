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

require 'ocm/argument'
require 'ocm/element'

class OCMMethod < OCMElement
  attr :arguments, true
  attr :result

  def initialize(arguments)
    super(arguments)
    @arguments = []
    @result = arguments.pop
  end

  def self.createFromXML(node, arguments = [])
    arguments.push(node["result"])

    method = super(node, arguments)
    node.xpath("argument").each do |a|
      method.arguments << OCMArgument.createFromXML(a)
    end

    return method
  end

  def to_s
    args = ""
    @arguments.each { |e| args = args + e.to_s + ", " }

    string = @result.empty? ? "procedure ".bold : "function ".bold
    string += @name
    string += ' ('.bold + args.chop.chop + ')'.bold unless args.empty?
    string += " return ".red + @result.blue unless @result.empty?
    return string
  end

  def eql?(m)
    return false if m == nil
    return super(m) && @arguments == m.arguments
  end

  def hash
    return [@name, @argument].hash
  end

  alias :== :eql?
end

