import java.util.Arrays;
import java.util.Random;
import java.util.Comparator;

public class Population {

	private Individual population[];
	private double populationFitness = -1;

	public Population(int populationSize) {
		this.population = new Individual[populationSize];
	}

	public Population(int populationSize, int chromosomeLength) {
		this.population = new Individual[populationSize];
		for (int individualCount = 0; individualCount < populationSize; individualCount++) {
			Individual individual = new Individual(chromosomeLength);
			this.population[individualCount] = individual;
		}
	}

	public Individual[] getIndividuals() {
		return this.population;
	}

	public Individual getFittest(int offset) {
		Arrays.sort(population, Comparator.comparing(Individual::getFitness));
		return population[offset];
	}

	public int size() {
		return population.length;
	}

	public Individual setIndividual(int offset, Individual individual) {
		return population[offset] = individual;
	}

	public Individual getIndividual(int offset) {
		return population[offset];
	}

	public void shuffle() {
		Random rnd = new Random();
		for (int i = population.length - 1; i > 0; i--) {
			int index = rnd.nextInt(i + 1);
			Individual a = population[index];
			population[index] = population[i];
			population[i] = a;
		}
	}
}