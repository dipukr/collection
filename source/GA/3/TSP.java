/***
 * 
 * Main, executive class for the Traveling Salesman Problem.
 * 
 * We don't have a real list of cities, so we randomly generate a number of them
 * on a 100x100 map.
 * 
 * The TSP requires that each city is visited once and only once, so we have to
 * be careful when initializing a random Individual and also when applying
 * crossover and mutation. Check out the GeneticAlgorithm class for
 * implementations of crossover and mutation for this problem.
 * 
 * @author bkanber
 *
***/

public class TSP {

	public static int maxGenerations = 10000;
	
	public static void main(String[] args) {
		int numCities = 100;
		City[] cities = new City[numCities];

		for (int cityIndex = 0; cityIndex < numCities; cityIndex++) {
			int x = (int) (100 * Math.random());
			int y = (int) (100 * Math.random());
			cities[cityIndex] = new City(x, y);
		}
		Algorithm ga = new Algorithm(100, 0.001, 0.9, 2, 5);
		Population population = ga.initPopulation(cities.length);
		ga.evalPopulation(population, cities);

		Route startRoute = new Route(population.getFittest(0), cities);
		System.out.println("Start Distance: "+startRoute.getDistance());
		int generation = 1;
		
		long start = System.currentTimeMillis();
		while (ga.isTerminationConditionMet(generation, maxGenerations) == false) {
			Route route = new Route(population.getFittest(0), cities);
			population = ga.crossoverPopulation(population);
			population = ga.mutatePopulation(population);
			ga.evalPopulation(population, cities);
			generation++;
		}
		long end = System.currentTimeMillis();
		System.out.println("Stopped after " + maxGenerations + " generations.");
		Route route = new Route(population.getFittest(0), cities);
		System.out.println("Best distance: " + route.getDistance());
		System.out.println("Total time (ms) elapsed: " + (end - start));
	}
}
