//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
network cluster
{     
    parameters:
        int Num @prompt("Number of Host") = default(15);
        double hellointerval @unit(s) = default(1s);
        @display("bgi=,s;bgb=1000,1000");
        volatile double posX = uniform(100,900);
        volatile double posY = uniform(100,900);
        //double posX = uniform(0,900);
        //double posY = uniform(0,900);
    submodules:
        Host[Num]: Host{
			parameters:
			    @display("p=$posX,$posY");        
        }
}
