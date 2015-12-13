
uniform sampler2DRect _tex0;
uniform float _width;
uniform float _height;
//uniform float _pRate;


// Fisheye to spherical conversion
// Assumes the fisheye image is square, centered, and the circle fills the image.
// Output (spherical) image should have 2:1 aspect.
// Strange (but helpful) that atan() == atan2(), normally they are different.

//kernel vec4 fish2sphere(sampler src)
vec2 fish2pano(vec2 texCoord)
{
  vec2 pfish;
  float theta,phi,r;
  vec3 psph;
  
  float FOV = 3.141592654 * 1.12; // FOV of the fisheye, eg: 180 degrees

  // Polar angles
  //float texCoordX = ( srcHeight*0.5 ) texCoord.x

  theta = 3.14159265 * (texCoord.x / _width - 0.5); // -pi to pi
  phi = 3.14159265 * (texCoord.y / _height - 0.5);  // -pi/2 to pi/2

  // Vector in 3D space
  psph.x = cos(phi) * sin(theta);
  psph.y = cos(phi) * cos(theta);
  psph.z = sin(phi);
  
  // Calculate fisheye angle and radius
  theta = atan(psph.z,psph.x);
  phi = atan(sqrt(psph.x*psph.x+psph.z*psph.z),psph.y);
  r = _width * phi / FOV; 

  // Pixel in fisheye space
  pfish.x = 0.5 * _width + r * cos(theta) ; //* 0.5;
  pfish.y = 0.5 * _height + r * sin(theta)  ;

  return pfish;
}

 
void main(void)//Drag mouse over rendering area
{
  //vec2 texCoord = vec2(pos.x, pos.y);
  vec2 texCoord = gl_TexCoord[0].xy;
  //vec4 col = texture2DRect(tex0, texCoord);

  vec4 col;
  if(texCoord.x < _width)
  {
	vec2 fish = fish2pano(texCoord);
	vec2 pos = vec2(fish.y,fish.x);
	col = texture2DRect(_tex0, pos);
  }
  else
  {
  	vec2 texCoord2 = vec2( texCoord.x - _width, texCoord.y );

	vec2 fish = fish2pano(texCoord2);
	vec2 pos = vec2((_width - fish.y) + _width,  _height - fish.x );
	col = texture2DRect(_tex0, pos);
  }
  
  gl_FragColor = col;
}