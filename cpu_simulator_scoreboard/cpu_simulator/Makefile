output: simulation.o pipeline.o pipe_component.o machine_code_generator.o global_data.o
	gcc simulation.o pipeline.o pipe_component.o machine_code_generator.o global_data.o -o simulation

simulation.o: simulation.c
	gcc -c simulation.c

pipeline.o: pipeline.c
	gcc -c pipeline.c

pipe_component.o: pipe_component.c
	gcc -c pipe_component.c

machine_code_generator.o: machine_code_generator.c
	gcc -c machine_code_generator.c

global_data.o: global_data.c
	gcc -c global_data.c

clean:
	rm *.o simulation