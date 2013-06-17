/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "evaluation_mx.hpp"
#include "../fx/fx_internal.hpp"
#include "../stl_vector_tools.hpp"
#include "../mx/mx_tools.hpp"
#include "../matrix/matrix_tools.hpp"

using namespace std;

namespace CasADi {

  EvaluationMX::EvaluationMX(const FX& fcn, std::vector<MX> arg) : fcn_(fcn) {

    // Number inputs and outputs
    int num_in = fcn.getNumInputs();
    casadi_assert(arg.size()<=num_in);

    // Add arguments if needed
    arg.resize(num_in);

    // Replace nulls with zeros of the right dimension
    for(int i=0; i<arg.size(); ++i){
      if(arg[i].isNull()) arg[i] = MX::zeros(fcn_.input(i).sparsity());
    }

    setDependencies(arg);
    setSparsity(CRSSparsity(1, 1, true));
  }

  EvaluationMX* EvaluationMX::clone() const {
    return new EvaluationMX(*this);
  }

  void EvaluationMX::printPart(std::ostream &stream, int part) const {
    if (part == 0) {
      stream << fcn_ << ".call([";
    } else if (part == ndep()) {
      stream << "])";
    } else {
      stream << ",";
    }
  }

  void EvaluationMX::evaluateD(const DMatrixPtrV& arg, DMatrixPtrV& res,
                               const DMatrixPtrVV& fseed, DMatrixPtrVV& fsens,
                               const DMatrixPtrVV& aseed, DMatrixPtrVV& asens) {
  
    // Number of inputs and outputs
    int num_in = fcn_.getNumInputs();
    int num_out = fcn_.getNumOutputs();

    // Number of derivative directions to calculate
    int nfdir = fsens.size();
    int nadir = aseed.size();

    // Number of derivative directions supported by the function
    int max_nfdir = fcn_.numAllocFwd();
    int max_nadir = fcn_.numAllocAdj();

    // Current forward and adjoint direction
    int offset_nfdir = 0, offset_nadir = 0;

    // Has the function been evaluated once
    bool fcn_evaluated = false;

    // Pass the inputs to the function
    for (int i = 0; i < num_in; ++i) {
      DMatrix *a = arg[i];
      if(a != 0){
        fcn_.setInput(*a, i);
      } else {
        fcn_.setInput(0., i);
      }
    }
  
    // Evaluate until everything has been determinated
    while (!fcn_evaluated || offset_nfdir < nfdir || offset_nadir < nadir) {

      // Number of forward and adjoint directions in the current "batch"
      int nfdir_f_batch = std::min(nfdir - offset_nfdir, max_nfdir);
      int nadir_f_batch = std::min(nadir - offset_nadir, max_nadir);

      // Pass the forward seeds to the function
      for(int d = 0; d < nfdir_f_batch; ++d){
        for(int i = 0; i < num_in; ++i){
          DMatrix *a = fseed[offset_nfdir + d][i];
          if(a != 0){
            fcn_.setFwdSeed(*a, i, d);
          } else {
            fcn_.setFwdSeed(0., i, d);
          }
        }
      }

      // Pass the adjoint seed to the function
      for(int d = 0; d < nadir_f_batch; ++d){
        for(int i = 0; i < num_out; ++i) {
          DMatrix *a = aseed[offset_nadir + d][i];
          if(a != 0){
            fcn_.setAdjSeed(*a, i, d);
          } else {
            fcn_.setAdjSeed(0., i, d);
          }
        }
      }

      // Evaluate
      fcn_.evaluate(nfdir_f_batch, nadir_f_batch);
    
      // Get the outputs if first evaluation
      if(!fcn_evaluated){
        for(int i = 0; i < num_out; ++i) {
          if(res[i] != 0) fcn_.getOutput(*res[i], i);
        }
      }

      // Marked as evaluated
      fcn_evaluated = true;

      // Get the forward sensitivities
      for(int d = 0; d < nfdir_f_batch; ++d){
        for(int i = 0; i < num_out; ++i) {
          DMatrix *a = fsens[offset_nfdir + d][i];
          if(a != 0) fcn_.getFwdSens(*a, i, d);
        }
      }

      // Get the adjoint sensitivities
      for (int d = 0; d < nadir_f_batch; ++d) {
        for (int i = 0; i < num_in; ++i) {
          DMatrix *a = asens[offset_nadir + d][i];
          if(a != 0){
            a->sparsity().add(a->ptr(),fcn_.adjSens(i,d).ptr(),fcn_.adjSens(i,d).sparsity());
          }
        }
      }

      // Update direction offsets
      offset_nfdir += nfdir_f_batch;
      offset_nadir += nadir_f_batch;
    }

    // Clear adjoint seeds
    clearVector(aseed);
  }

  int EvaluationMX::getNumOutputs() const {
    return fcn_.getNumOutputs();
  }

  const CRSSparsity& EvaluationMX::sparsity(int oind) const{
    return fcn_.output(oind).sparsity();
  }

  FX& EvaluationMX::getFunction() {
    return fcn_;
  }

  void EvaluationMX::evaluateSX(const SXMatrixPtrV& arg, SXMatrixPtrV& res,
                                const SXMatrixPtrVV& fseed, SXMatrixPtrVV& fsens,
                                const SXMatrixPtrVV& aseed, SXMatrixPtrVV& asens) {
  
    // Create input arguments
    vector<SXMatrix> argv(arg.size());
    for(int i=0; i<arg.size(); ++i){
      argv[i] = SXMatrix(fcn_.input(i).sparsity(),0.);
      if(arg[i] != 0)
        argv[i].set(*arg[i]);
    }

    // Evaluate symbolically
    vector<SXMatrix> resv = fcn_.eval(argv);

    // Collect the result
    for (int i = 0; i < res.size(); ++i) {
      if (res[i] != 0)
        *res[i] = resv[i];
    }
  }

  void EvaluationMX::evaluateMX(const MXPtrV& input, MXPtrV& output, const MXPtrVV& fwdSeed, MXPtrVV& fwdSens, const MXPtrVV& adjSeed, MXPtrVV& adjSens, bool output_given) {
    // Collect inputs and seeds
    vector<MX> arg = getVector(input);
    vector<vector<MX> > fseed = getVector(fwdSeed);
    vector<vector<MX> > aseed = getVector(adjSeed);

    // Free adjoint seeds
    clearVector(adjSeed);

    // Evaluate symbolically
    vector<MX> res;
    vector<vector<MX> > fsens, asens;
    fcn_->createCallDerivative(arg,res,fseed,fsens,aseed,asens,true);

    // Store the non-differentiated results
    if(!output_given){
      for(int i=0; i<res.size(); ++i){
        if(output[i]!=0){
          *output[i] = res[i];
        }
      }
    }

    // Store the forward sensitivities
    for(int d=0; d<fwdSens.size(); ++d){
      for(int i=0; i<fwdSens[d].size(); ++i){
        if(fwdSens[d][i]!=0){
          *fwdSens[d][i] = fsens[d][i];
        }
      }
    }

    // Store the adjoint sensitivities
    for(int d=0; d<adjSens.size(); ++d){
      for(int i=0; i<adjSens[d].size(); ++i){
        if(adjSens[d][i]!=0 && !asens[d][i].isNull()){
          *adjSens[d][i] += asens[d][i];
        }
      }
    }
  }

  void EvaluationMX::deepCopyMembers(std::map<SharedObjectNode*, SharedObject>& already_copied) {
    MXNode::deepCopyMembers(already_copied);
    fcn_ = deepcopy(fcn_, already_copied);
  }

  void EvaluationMX::propagateSparsity(DMatrixPtrV& arg, DMatrixPtrV& res, bool use_fwd) {
    // Pass/clear forward seeds/adjoint sensitivities
    for (int iind = 0; iind < fcn_.getNumInputs(); ++iind) {
      // Input vector
      vector<double> &v = fcn_.input(iind).data();
      if (v.empty()) continue; // FIXME: remove?
      
      if (arg[iind] == 0) {
        // Set to zero if not used
        fill_n(get_bvec_t(v), v.size(), bvec_t(0));
      } else {
        // Copy output
        fcn_.input(iind).sparsity().set(get_bvec_t(fcn_.input(iind).data()),get_bvec_t(arg[iind]->data()),arg[iind]->sparsity());
      }
    }
    
    // Pass/clear adjoint seeds/forward sensitivities
    for (int oind = 0; oind < fcn_.getNumOutputs(); ++oind) {
      // Output vector
      vector<double> &v = fcn_.output(oind).data();
      if (v.empty()) continue; // FIXME: remove?
      
      if (res[oind] == 0) {
        // Set to zero if not used
        fill_n(get_bvec_t(v), v.size(), bvec_t(0));
      } else {
        // Copy output
        fcn_.output(oind).sparsity().set(get_bvec_t(fcn_.output(oind).data()),get_bvec_t(res[oind]->data()),res[oind]->sparsity());
        if(!use_fwd) fill_n(get_bvec_t(res[oind]->data()),res[oind]->size(),bvec_t(0));
      }
    }
    
    // Propagate seeds
    fcn_.spInit(use_fwd); // NOTE: should only be done once
    if(fcn_.spCanEvaluate(use_fwd)){
      fcn_.spEvaluate(use_fwd);
    } else {
      fcn_->spEvaluateViaJacSparsity(use_fwd);
    }
    
    // Get the sensitivities
    if (use_fwd) {
      for (int oind = 0; oind < res.size(); ++oind) {
        if (res[oind] != 0) {
          res[oind]->sparsity().set(get_bvec_t(res[oind]->data()),get_bvec_t(fcn_.output(oind).data()),fcn_.output(oind).sparsity());
        }
      }
    } else {
      for (int iind = 0; iind < arg.size(); ++iind) {
        if (arg[iind] != 0) {
          arg[iind]->sparsity().bor(get_bvec_t(arg[iind]->data()),get_bvec_t(fcn_.input(iind).data()),fcn_.input(iind).sparsity());
        }
      }
    }

    // Clear seeds and sensitivities
    for (int iind = 0; iind < arg.size(); ++iind) {
      vector<double> &v = fcn_.input(iind).data();
      fill(v.begin(), v.end(), 0);
    }
    for (int oind = 0; oind < res.size(); ++oind) {
      vector<double> &v = fcn_.output(oind).data();
      fill(v.begin(), v.end(), 0);
    }
  }

  void EvaluationMX::generateOperation(std::ostream &stream, const std::vector<std::string>& arg, const std::vector<std::string>& res, CodeGenerator& gen) const{
  
    // Running index of the temporary used
    int nr=0;

    // Copy arguments with nonmatching sparsities to the temp vector
    vector<string> arg_mod = arg;
    for(int i=0; i<fcn_.getNumInputs(); ++i){
      if(dep(i).sparsity()!=fcn_.input(i).sparsity()){
        arg_mod[i] = "rrr+" + CodeGenerator::numToString(nr);
        nr += fcn_.input(i).size();
        
        // Codegen "copy sparse"
        gen.addAuxiliary(CodeGenerator::AUX_COPY_SPARSE);
        
        int sp_arg = gen.getSparsity(dep(i).sparsity());
        int sp_input = gen.addSparsity(fcn_.input(i).sparsity());
        stream << "  casadi_copy_sparse(" << arg[i] << ",s" << sp_arg << "," << arg_mod[i] << ",s" << sp_input << ");" << std::endl;
      }
    }

    // Get the index of the function
    int f = gen.getDependency(fcn_);
    stream << "  f" << f << "(";
  
    // Pass inputs to the function input buffers
    for(int i=0; i<arg.size(); ++i){
      stream << arg_mod.at(i);
      if(i+1<arg.size()+res.size()) stream << ",";
    }

    // Separate arguments and results with an extra space
    stream << " ";

    // Pass results to the function input buffers
    for(int i=0; i<res.size(); ++i){
      stream << res.at(i);
      if(i+1<res.size()) stream << ",";
    }
  
    // Finalize the function call
    stream << ");" << endl;  
  }
  
  void EvaluationMX::nTmp(size_t& ni, size_t& nr){
    // Start with no extra memory
    ni=0;
    nr=0;

    // Add memory for all inputs with nonmatching sparsity
    for(int i=0; i<fcn_.getNumInputs(); ++i){
      if(dep(i).isNull() || dep(i).sparsity()!=fcn_.input(i).sparsity()){
        nr += fcn_.input(i).size();
      }
    }
  }

} // namespace CasADi
