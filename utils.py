#@+leo-ver=4-thin
#@+node:gcross.20090930124443.1265:@thin utils.py
#@@language Python

#@+others
#@+node:gcross.20090930124443.1249:Macro building functions
#@+at
# These routines build macros to perform tensor contractions.  They do this
# by construction a string of Python code which performs the contraction,
# and then compiling the code into a function.
#@-at
#@@c

#@+others
#@+node:gcross.20090930124443.1250:n2l
#@+at
# Utility function converting numbers to letters.
#@-at
#@@c
n2l = map(chr,range(ord('A'),ord('Z')+1))
#@-node:gcross.20090930124443.1250:n2l
#@+node:gcross.20090930124443.1251:make_contractor
def make_contractor(tensor_index_labels,index_join_pairs,result_index_labels,name="f"):    # pre-process parameters
    tensor_index_labels = list(map(list,tensor_index_labels))
    index_join_pairs = list(index_join_pairs)
    result_index_labels = list([list(index_group) if hasattr(index_group,"__getitem__") else [index_group] for index_group in result_index_labels])

    assert sum(len(index_group) for index_group in tensor_index_labels) == (sum(len(index_group) for index_group in result_index_labels)+2*len(index_join_pairs))

    function_definition_statements = ["def %s(%s):" % (name,",".join(n2l[:len(tensor_index_labels)]))]

    #@    << def build_statements >>
    #@+node:gcross.20090930124443.1252:<< def build_statements >>
    def build_statements(tensor_index_labels,index_join_pairs,result_index_labels):
    #@+at
    # This routine recursively builds a list of statements which performs the 
    # full tensor contraction.
    # 
    # First, if there is only one tensor left, then transpose and reshape it 
    # to match the result_index_labels.
    #@-at
    #@@c
        if len(tensor_index_labels) == 1:
            if len(result_index_labels) == 0:
                return ["return A"]
            else:
                final_index_labels = tensor_index_labels[0]
                result_indices = [[final_index_labels.index(index) for index in index_group] for index_group in result_index_labels]
                transposed_indices = __builtin__.sum(result_indices,[])
                assert type(transposed_indices) == list
                assert len(final_index_labels) == len(transposed_indices)
                new_shape = ",".join(["(%s)" % "*".join(["shape[%i]"%index for index in index_group]) for index_group in result_indices])     
                return ["shape=A.shape","return A.transpose(%s).reshape(%s)" % (transposed_indices,new_shape)]
    #@+at
    # Second, if all joins have finished, then take outer products to combine 
    # all remaining tensors into one.
    #@-at
    #@@c
        elif len(index_join_pairs) == 0:
            if tensor_index_labels[-1] is None:
                return build_statements(tensor_index_labels[:-1],index_join_pairs,result_index_labels)
            elif len(tensor_index_labels[-1]) == 0:
                v = n2l[len(tensor_index_labels)-1]
                return ["A*=%s" % v, "del %s" % v] + build_statements(tensor_index_labels[:-1],index_join_pairs,result_index_labels)
            else:
                v = n2l[len(tensor_index_labels)-1]
                tensor_index_labels[0] += tensor_index_labels[-1]
                return ["A = multiply.outer(A,%s)" % v, "del %s" % v] + build_statements(tensor_index_labels[:-1],index_join_pairs,result_index_labels)
    #@+at
    # Otherwise, do the first join, walking through index_join_pairs to find 
    # any other pairs which connect the same two tensors.
    #@-at
    #@@c
        else:
            #@        << Search for all joins between these tensors >>
            #@+node:gcross.20090930124443.1253:<< Search for all joins between these tensors >>
            #@+at
            # This function searches for the tensors which are joined, and 
            # reorders the indices in the join so that the index corresponding 
            # to the tensor appearing first in the list of tensors appears 
            # first in the join.
            #@-at
            #@@c
            def find_tensor_ids(join):
                reordered_join = [None,None]
                tensor_ids = [0,0]
                join = list(join)
                while tensor_ids[0] < len(tensor_index_labels):
                    index_labels = tensor_index_labels[tensor_ids[0]]
                    if index_labels is None:
                        tensor_ids[0] += 1
                    elif join[0] in index_labels:
                        reordered_join[0] = index_labels.index(join[0])
                        del join[0]
                        break
                    elif join[1] in index_labels:
                        reordered_join[0] = index_labels.index(join[1])
                        del join[1]
                        break
                    else:
                        tensor_ids[0] += 1
                assert len(join) == 1 # otherwise index was not found in any tensor
                tensor_ids[1] = tensor_ids[0] + 1
                while tensor_ids[1] < len(tensor_index_labels):
                    index_labels = tensor_index_labels[tensor_ids[1]]
                    if index_labels is None:
                        tensor_ids[1] += 1
                    elif join[0] in index_labels:
                        reordered_join[reordered_join.index(None)] = index_labels.index(join[0])
                        del join[0]
                        break
                    else:
                        tensor_ids[1] += 1
                assert len(join) == 0 # otherwise index was not found in any tensor
                return tensor_ids, reordered_join

            join_indices = [0]
            tensor_ids,reordered_join = find_tensor_ids(index_join_pairs[0])

            indices = [[],[]]

            for j in xrange(2):
                indices[j].append(reordered_join[j])

            # Search for other joins between these tensors
            for i in xrange(1,len(index_join_pairs)):
                tensor_ids_,reordered_join = find_tensor_ids(index_join_pairs[i])
                if tensor_ids == tensor_ids_:
                    join_indices.append(i)
                    for j in xrange(2):
                        indices[j].append(reordered_join[j])

            #@-node:gcross.20090930124443.1253:<< Search for all joins between these tensors >>
            #@nl

            #@        << Build tensor contraction statements >>
            #@+node:gcross.20090930124443.1254:<< Build tensor contraction statements >>
            tensor_vars = [n2l[id] for id in tensor_ids]

            statements = [
                "try:",
                "   %s = tensordot(%s,%s,%s)" % (tensor_vars[0],tensor_vars[0],tensor_vars[1],indices),
                "   del %s" % tensor_vars[1],
                "except ValueError:",
                "   raise ValueError('indices %%s do not match for tensor %%i, shape %%s, and tensor %%i, shape %%s.' %% (%s,%i,%s.shape,%i,%s.shape))" % (indices,tensor_ids[0],tensor_vars[0],tensor_ids[1],tensor_vars[1])
            ]
            #@-node:gcross.20090930124443.1254:<< Build tensor contraction statements >>
            #@nl

            #@        << Delete joins from list and update tensor specifications >>
            #@+node:gcross.20090930124443.1255:<< Delete joins from list and update tensor specifications >>
            join_indices.reverse()
            for join_index in join_indices:
                del index_join_pairs[join_index]

            new_tensor_index_labels_0 = list(tensor_index_labels[tensor_ids[0]])
            indices[0].sort(reverse=True)
            for index in indices[0]:
                del new_tensor_index_labels_0[index]

            new_tensor_index_labels_1 = list(tensor_index_labels[tensor_ids[1]])
            indices[1].sort(reverse=True)
            for index in indices[1]:
                del new_tensor_index_labels_1[index]

            tensor_index_labels[tensor_ids[0]] = new_tensor_index_labels_0+new_tensor_index_labels_1
            tensor_index_labels[tensor_ids[1]] = None
            #@-node:gcross.20090930124443.1255:<< Delete joins from list and update tensor specifications >>
            #@nl

            return statements + build_statements(tensor_index_labels,index_join_pairs,result_index_labels)
    #@-node:gcross.20090930124443.1252:<< def build_statements >>
    #@nl

    function_definition_statements += ["\t" + statement for statement in build_statements(tensor_index_labels,index_join_pairs,result_index_labels)]

    function_definition = "\n".join(function_definition_statements)+"\n"

    f_globals = {"tensordot":tensordot,"multiply":multiply}
    f_locals = {}

    exec function_definition in f_globals, f_locals

    f = f_locals[name]
    f.source = function_definition
    return f
#@nonl
#@-node:gcross.20090930124443.1251:make_contractor
#@+node:gcross.20090930124443.1256:make_contractor_from_implicit_joins
def make_contractor_from_implicit_joins(tensor_index_labels,result_index_labels,name="f"):
    tensor_index_labels = list(map(list,tensor_index_labels))
    found_indices = {}
    index_join_pairs = []
    for i in xrange(len(tensor_index_labels)):
        for index_position, index in enumerate(tensor_index_labels[i]):
            if index in found_indices:
                other_tensor = found_indices[index]
                if other_tensor is None:
                    raise ValueError("index label %s found in more than two tensors" % index)
                else:
                    # rename this instance of the index and add to the list of join pairs
                    tensor_index_labels[i][index_position] = (i,index)
                    index_join_pairs.append((index,(i,index)))
                    # mark that we have found two instances of this index for
                    # error-checking purposes
                    found_indices[index] = None
            else:
                found_indices[index] = i
    return make_contractor(tensor_index_labels,index_join_pairs,result_index_labels,name)
#@nonl
#@-node:gcross.20090930124443.1256:make_contractor_from_implicit_joins
#@-others
#@-node:gcross.20090930124443.1249:Macro building functions
#@+node:gcross.20090930124443.1275:Unit Tests
if __name__ == '__main__':
    import unittest
    import __builtin__
    from numpy import allclose, tensordot, multiply, dot, inner
    from numpy.random import randint, rand
    #@    @+others
    #@+node:gcross.20090930124443.1276:make_contractor_tests
    class make_contractor_tests(unittest.TestCase):
        #@    @+others
        #@+node:gcross.20090930124443.1277:testIdentity
        def testIdentity(self):
            arr = rand(3,4,5)
            self.assertTrue(allclose(arr,make_contractor([[0,1,2]],[],[[0],[1],[2]])(arr),rtol=1e-10))
            self.assertTrue(allclose(arr.ravel(),make_contractor([[0,1,2]],[],[[0,1,2]])(arr),rtol=1e-10))
            self.assertTrue(allclose(arr.transpose(2,0,1).ravel(),make_contractor([[0,1,2]],[],[[2,0,1]])(arr),rtol=1e-10))
            self.assertTrue(allclose(arr.transpose(2,0,1).reshape(15,4),make_contractor([[0,1,2]],[],[[2,0],[1]])(arr),rtol=1e-10))
        #@nonl
        #@-node:gcross.20090930124443.1277:testIdentity
        #@+node:gcross.20090930124443.1278:testInnerProduct
        def testInnerProduct(self):
            A = rand(3,5)
            B = rand(5,3)
            MP = make_contractor([[0,1],[2,3]],[[1,2]],[0,3])
            self.assertTrue(allclose(dot(A,B),MP(A,B),rtol=1e-10))
            self.assertTrue(allclose(dot(B,A),MP(B,A),rtol=1e-10))
            TP = make_contractor([[0,1],[2,3]],[[1,2],[3,0]],[])
            self.assertTrue(allclose(inner(A.transpose().ravel(),B.ravel()),TP(A,B),rtol=1e-10))
        #@nonl
        #@-node:gcross.20090930124443.1278:testInnerProduct
        #@+node:gcross.20090930124443.1279:testImplicitJoins
        def testImplicitJoins(self):
            arr = rand(3,4,5)
            self.assertTrue(allclose(arr,make_contractor_from_implicit_joins([[0,1,2]],[[0],[1],[2]])(arr),rtol=1e-10))
            self.assertTrue(allclose(arr.ravel(),make_contractor_from_implicit_joins([[0,1,2]],[[0,1,2]])(arr),rtol=1e-10))
            self.assertTrue(allclose(arr.transpose(2,0,1).ravel(),make_contractor_from_implicit_joins([[0,1,2]],[[2,0,1]])(arr),rtol=1e-10))
            self.assertTrue(allclose(arr.transpose(2,0,1).reshape(15,4),make_contractor_from_implicit_joins([[0,1,2]],[[2,0],[1]])(arr),rtol=1e-10))

            A = rand(3,5)
            B = rand(5,3)
            MP = make_contractor_from_implicit_joins([[0,1],[1,3]],[[0],[3]])
            self.assertTrue(allclose(dot(A,B),MP(A,B),rtol=1e-10))
            self.assertTrue(allclose(dot(B,A),MP(B,A),rtol=1e-10))
            TP = make_contractor_from_implicit_joins([[0,1],[1,0]],[])
            self.assertTrue(allclose(inner(A.transpose().ravel(),B.ravel()),TP(A,B),rtol=1e-10))

            arr = rand(3,4,5)
            self.assertTrue(allclose(arr*5,make_contractor_from_implicit_joins([[0,1,2],[]],[[0],[1],[2]])(arr,5),rtol=1e-10))
            self.assertTrue(allclose(arr*5*6,make_contractor_from_implicit_joins([[0,1,2],[],[]],[[0],[1],[2]])(arr,5,6),rtol=1e-10))
            self.assertTrue(allclose(multiply.outer(arr,arr),make_contractor_from_implicit_joins([[0,1,2],[3,4,5]],[[i] for i in xrange(6)])(arr,arr),rtol=1e-10))
            self.assertTrue(allclose(reduce(multiply.outer,[arr,]*3),make_contractor_from_implicit_joins([[0,1,2],[3,4,5],[6,7,8]],[[i] for i in xrange(9)])(arr,arr,arr),rtol=1e-10))
            A = rand(3,4,5)
            B = rand(3,4,5)
            C = rand(3,4,5)
            self.assertTrue(allclose(reduce(multiply.outer,[A,B,C]),make_contractor_from_implicit_joins([[0,1,2],[3,4,5],[6,7,8]],[[i] for i in xrange(9)])(A,B,C),rtol=1e-10))
        #@nonl
        #@-node:gcross.20090930124443.1279:testImplicitJoins
        #@+node:gcross.20090930124443.1280:testOuterProduct
        def testOuterProduct(self):
            arr = rand(3,4,5)
            self.assertTrue(allclose(arr*5,make_contractor([[0,1,2],[]],[],range(3))(arr,5),rtol=1e-10))
            self.assertTrue(allclose(arr*5*6,make_contractor([[0,1,2],[],[]],[],range(3))(arr,5,6),rtol=1e-10))
            self.assertTrue(allclose(multiply.outer(arr,arr),make_contractor([[0,1,2],[3,4,5]],[],range(6))(arr,arr),rtol=1e-10))
            self.assertTrue(allclose(reduce(multiply.outer,[arr,]*3),make_contractor([[0,1,2],[3,4,5],[6,7,8]],[],range(9))(arr,arr,arr),rtol=1e-10))
            A = rand(3,4,5)
            B = rand(3,4,5)
            C = rand(3,4,5)
            self.assertTrue(allclose(reduce(multiply.outer,[A,B,C]),make_contractor([[0,1,2],[3,4,5],[6,7,8]],[],[[i] for i in xrange(9)])(A,B,C),rtol=1e-10))
        #@nonl
        #@-node:gcross.20090930124443.1280:testOuterProduct
        #@-others
    #@nonl
    #@-node:gcross.20090930124443.1276:make_contractor_tests
    #@-others
    unittest.main()
#@-node:gcross.20090930124443.1275:Unit Tests
#@-others
#@-node:gcross.20090930124443.1265:@thin utils.py
#@-leo
