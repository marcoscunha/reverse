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

class OCMSet < Hash

  SECTIONS = [ 'type', 'definition', 'variable', 'method' ]

  def self.new(*arguments)
    h = super(*arguments)
    SECTIONS.each { |s| h.store(s, []) }
    return h
  end

  def overlap?(set)
    SECTIONS.each do |key, value|
      union = (set[key] + self[key]).uniq
      length = self[key].length + set[key].length
      return true if union.length < length
    end

    return false
  end

end
