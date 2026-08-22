public class City {
	
	private int x;
	private int y;

	public City(int x, int y) {
		this.x = x;
		this.y = y;
	}

	public double distanceFrom(City city) {
		double deltaXSq = Math.pow((city.x - this.x), 2);
		double deltaYSq = Math.pow((city.y - this.y), 2);
		double distance = Math.sqrt(Math.abs(deltaXSq + deltaYSq));
		return distance;
	}
}
