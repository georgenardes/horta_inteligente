package com.example.horta_inteligente;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.SwitchCompat;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.util.logging.Logger;

public class MainActivity extends AppCompatActivity {

    /*
    * Define o modo de operação atual
    * 0 = manual
    * 1 = automático
    * */
    int modo_operacao = 0;

    EditText etTemperaturaBase;
    EditText etLuminosidadeBase;
    EditText etUmidadeBase;

    // atuadores
    TextView tvBomba;
    TextView tvLampada;
    TextView tvVentiladr;

    // sensores
    TextView tvLuminosidade;
    TextView tvNivel;
    TextView tvTemperatura;
    TextView tvUmidade;

    // switchs
    Switch swModo;
    Switch swVentilador;
    Switch swBomba;
    Switch swLampada;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // referêcias de campo de texto
        etTemperaturaBase = findViewById(R.id.etTemperaturaBase);
        etLuminosidadeBase = findViewById(R.id.etLuminosidadeBase);
        etUmidadeBase = findViewById(R.id.etUmidadeBase);


        // atuadores
        tvBomba = findViewById(R.id.tvAtBomba);
        tvLampada = findViewById(R.id.tvAtLampada);
        tvVentiladr = findViewById(R.id.tvAtVentilador);

        // sensores
        tvLuminosidade = findViewById(R.id.tvSensLuminosidade);
        tvNivel = findViewById(R.id.tvSensNivel);
        tvTemperatura = findViewById(R.id.tvSenTemperatura);
        tvUmidade = findViewById(R.id.tvSenUmidade);

        // comandos
        swModo = findViewById(R.id.switchModo);
        swVentilador = findViewById(R.id.switchVentilador);
        swBomba = findViewById(R.id.switchBomba);
        swLampada = findViewById(R.id.switchLampada);


        // faz binding dos componentes visuais com os valores do banco
        binding();
    }

    private void binding(){
        // referencia ao banco e as variaveis
        final FirebaseDatabase database = FirebaseDatabase.getInstance();

        // monitoramento bomba
        // =======================================================
        DatabaseReference dbrEstadoAtBomba = database.getReference(
                "estado/atuadores/bomba_agua"
        );
        dbrEstadoAtBomba.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                int value = snapshot.getValue(Integer.class);

                if (value == 1){
                    tvBomba.setText("ativado");
                }else{
                    tvBomba.setText("desativado");
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                // falha ao ler valor
            }
        });
        // ========================================================

        // monitoramento lampada
        // =======================================================
        DatabaseReference dbrEstadoAtLampada = database.getReference(
                "estado/atuadores/lampada"
        );
        dbrEstadoAtLampada.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                int value = snapshot.getValue(Integer.class);

                if (value == 1){
                    tvLampada.setText("ativado");
                }else{
                    tvLampada.setText("desativado");
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });
        // ========================================================

        // monitoramento ventilador
        // =======================================================
        DatabaseReference dbrEstadoAtVentialdor = database.getReference(
                "estado/atuadores/ventilador"
        );
        dbrEstadoAtVentialdor.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                int value = snapshot.getValue(Integer.class);

                if (value == 1){
                    tvVentiladr.setText("ativado");
                }else{
                    tvVentiladr.setText("desativado");
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });
        // ========================================================


        // monitoramento sensor Luminosidade
        // =======================================================
        DatabaseReference dbrEstadoSenLuminosidade = database.getReference(
                "estado/sensores/luminosidade"
        );
        dbrEstadoSenLuminosidade.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                int value = snapshot.getValue(Integer.class);
                tvLuminosidade.setText(Integer.toString(value));
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });
        // ========================================================


        // monitoramento sensor nivel
        // =======================================================
        DatabaseReference dbrEstadoSenNivel = database.getReference(
                "estado/sensores/nivel"
        );
        dbrEstadoSenNivel.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                int value = snapshot.getValue(Integer.class);

                if (value == 1)
                    tvNivel.setText("Com água");
                else
                    tvNivel.setText("Sem água");
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });
        // ========================================================

        // monitoramento sensor temperatura
        // =======================================================
        DatabaseReference dbrEstadoSenTemperatura = database.getReference(
                "estado/sensores/temperatura"
        );
        dbrEstadoSenTemperatura.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                int value = snapshot.getValue(Integer.class);
                tvTemperatura.setText(Integer.toString(value));
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });
        // ========================================================

        // monitoramento sensor umidade
        // =======================================================
        DatabaseReference dbrEstadoSenUmidade = database.getReference(
                "estado/sensores/umidade"
        );
        dbrEstadoSenUmidade.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                int value = snapshot.getValue(Integer.class);
                tvUmidade.setText(Integer.toString(value));
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {

            }
        });
        // ========================================================

    }


    public void onClickSalvar(View v){

        // referencia ao banco e as variaveis
        FirebaseDatabase database = FirebaseDatabase.getInstance();
        DatabaseReference dbrConfigLuminosidade = database.getReference(
                "configuracoes/luminosidade"
        );
        DatabaseReference dbrConfigTemperatura = database.getReference(
                "configuracoes/temperatura"
        );
        DatabaseReference dbrConfigUmidade = database.getReference(
                "configuracoes/umidade"
        );

        // pega os valores dos objetos visuais
        int iLuminosidadeBase = Integer.parseInt(etLuminosidadeBase.getText().toString().trim());
        int iTemperaturaBase = Integer.parseInt(etTemperaturaBase.getText().toString().trim());
        int iUmidadebase = Integer.parseInt(etUmidadeBase.getText().toString().trim());

        // insere valores no banco
        dbrConfigLuminosidade.setValue(iLuminosidadeBase);
        dbrConfigTemperatura.setValue(iTemperaturaBase);
        dbrConfigUmidade.setValue(iUmidadebase);
    }

    public void onSwitch(View v){

        // referencia ao banco e as variaveis
        FirebaseDatabase database = FirebaseDatabase.getInstance();
        DatabaseReference dbrModo = database.getReference(
                "modo"
        );
        DatabaseReference dbrContBomba = database.getReference(
                "controle/bomba_agua"
        );
        DatabaseReference dbrContLamp = database.getReference(
                "controle/lampada"
        );
        DatabaseReference dbrContVentilador = database.getReference(
                "controle/ventilador"
        );


        switch (v.getId()){
            case R.id.switchModo:
                if (swModo.isChecked()){
                    dbrModo.setValue(1);
                } else {
                    dbrModo.setValue(0);
                }
                break;
            case R.id.switchBomba:
                if (swBomba.isChecked()){
                    dbrContBomba.setValue(1);
                } else {
                    dbrContBomba.setValue(0);
                }

                break;
            case R.id.switchLampada:
                if (swLampada.isChecked()){
                    dbrContLamp.setValue(1);
                } else {
                    dbrContLamp.setValue(0);
                }
                break;
            case R.id.switchVentilador:
                if (swVentilador.isChecked()){
                    dbrContVentilador.setValue(1);
                } else {
                    dbrContVentilador.setValue(0);
                }
                break;
            default:

        }




    }

}