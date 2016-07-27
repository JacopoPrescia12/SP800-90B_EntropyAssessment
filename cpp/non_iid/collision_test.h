#include "../shared/utils.h"

// Builds a vector of the distance between collisions
vector<int> find_collisions(const byte data[]){
	vector<int> ret;
	set<int> dups;

	unsigned long int i = 0;
	unsigned long int check_size;

	// Begin from each element
	while(i < SIZE){

		check_size = 0;

		// Progressively increase the number of elements checked
		while(check_size < (SIZE - i)){

			// Toss elements into a set
			dups.insert(data[i+check_size]);

			// If sizes don't match up then a collision exists
			if(dups.size() != check_size+1){

				// Record info on collision and end inner loop
				// Advance outer loop past the collision end
				ret.push_back(check_size+1);
				i += check_size;
				check_size = SIZE;
				dups.clear();
			}

			check_size++;
		}

		i++;
	}

	return ret;
}

// Averages (mean) the vector
double avg_collision(const vector<int> &col_seq){

	return sum(col_seq)/float(col_seq.size());
}

// Derived from http://dlmf.nist.gov/8.9
double F(double z_inv, int k){
	double z = 1.0 / z_inv;
	double denom = 1.0 + (k / z);

	for(int i = 1; i < k; i++){
		denom = z - (i / denom);
		denom = 1.0 + ((k-i) / denom);
	}

	denom = z - (k / denom);
	return (1.0 / denom);
}

// Expected value of statistic based on one-parameter family of probability distributions
// Right hand side of equation in step 9 of Section 6.3.2
double calc_EpS(double p, int k){
	double q = (1.0 - p)/ (double)(k - 1.0);
	double p_inv = 1.0 / p;
	double q_inv = 1.0 / q;
	double q_inv_sq = q_inv * q_inv;
	double pq_k = (1.0 / k) * (p_inv - q_inv);

	return (p * q_inv_sq * (1.0 + pq_k) * F(q, k)) - (p * q_inv * pq_k);
}

// Run a binary search for the correct value of p
bool binary_search(const double lbound, const int n, double &p){

	double min_p = 1.0 / (double)n;
	double p_c = (1 - min_p) / 2.0 + min_p;
	double adj = 1 - min_p;
	double E_p = calc_EpS(p_c, n);
	double E_p_maxvalid = calc_EpS(min_p, n);

	cout << "E_p: " << E_p << endl;
	cout << "Max E_p: " << E_p_maxvalid << endl;
	if(lbound > E_p_maxvalid){
		p = 0.0;
		cout << "Bad bound" << endl;
		return false;
	}


	// Search until error is minimal
	while(abs(lbound - E_p) > .0001){
		
		adj /= 2.0;
		
		if(lbound < E_p){
			p_c += adj;
		
			// Avoid dividing by 0 in later steps
			if(p_c == 1.0){
				p_c -= .0001;
			}
		}else{
			p_c -= adj;
			
			// Avoid dipping below the minimum
			if(p_c < min_p){
				p_c = min_p;
			}
		}

		E_p = calc_EpS(p_c, n);
	}

	p = p_c;
	return true;
}

double collision_test(const byte data[]){
	
	vector<int> t = find_collisions(data);
	int v = t.size();

	if(v < 1000){
		cout << "v is less than 1000" << endl;
		return 0.0;
	}

	double t_mean = avg_collision(t);
	double t_stddev = std_dev(t, t_mean);

	double lower_bound = t_mean - 2.576 * (t_stddev / sqrt(v));

	cout << "Lower bound: " << lower_bound << endl;

	double p = 0.0;
	if(binary_search(lower_bound, 256, p)){
		return -log2(p);
	}else{
		cout << "Error in binary search" << endl;
		return log2(256);
	}
}