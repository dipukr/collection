public class Individual {
	
	private int[] chromosome;
	private double fitness = -1;

	public Individual(int[] chromosome) {
		this.chromosome = chromosome;
	}

	public Individual(int chromosomeLength) {
		int[] individual = new int[chromosomeLength];
		for (int gene = 0; gene < chromosomeLength; gene++)
			individual[gene] = gene;	
		this.chromosome = individual;
	}

	public String toString() {
		String output = "";
		for (int gene = 0; gene < this.chromosome.length; gene++)
			output += chromosome[gene] + ",";
		return output;
	}

	public boolean containsGene(int gene) {
		for (int i = 0; i < this.chromosome.length; i++)
			if (this.chromosome[i] == gene)
				return true;
		return false;
	}
}
